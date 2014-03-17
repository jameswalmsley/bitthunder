#include <bitthunder.h>
#include <collections/bt_list.h>
#include <interrupts/bt_tasklets.h>
#include <interfaces/bt_dev_if_spi.h>
#include <string.h>
#include <of/bt_of.h>

BT_DEF_MODULE_NAME			("BT SPI Manager")
BT_DEF_MODULE_DESCRIPTION	("Manages SPI Master busses, and handles SPI device probing")
BT_DEF_MODULE_AUTHOR		("Micheal Daniel")
BT_DEF_MODULE_EMAIL			("mdaniel@riegl.com")

static BT_LIST_HEAD(g_spi_masters);

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

struct spi_bus_item {
	struct bt_list_head bus_list;
	BT_HANDLE 			hMaster;
	BT_SPI_MASTER 	   *pMaster;
	BT_u32				sm_flags;
	#define SM_FLAG_PROBE	1
	void *bus_mutex;
	//bt_spinlock_t bus_spinlock;
};

static void spi_probe_devices(struct spi_bus_item *master) {
	BT_u32 i;
	BT_ERROR Error;

	BT_kPrint("spi_probe_devices called");

	BT_u32 ulTotalDevices = BT_GetTotalDevicesByType(BT_DEVICE_SPI);
	for(i = 0; i < ulTotalDevices; i++) {
		const BT_DEVICE *pDevice = BT_GetDeviceByType(BT_DEVICE_SPI, i);
		if(!pDevice) {
			continue;
		}

		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pDevice->name);
		if(!pDriver || pDriver->eType != BT_DRIVER_SPI) {
			continue;
		}

		const BT_RESOURCE *pResource = BT_GetResource(pDevice->pResources, pDevice->ulTotalResources, BT_RESOURCE_BUSID, 0);
		if(!pResource || pResource->ulStart != master->pMaster->bus_num) {
			continue;
		}

		BT_SPI_DEVICE *pSpiDevice = BT_kMalloc(sizeof(*pSpiDevice));
		pSpiDevice->pMaster = master->pMaster;

		pDriver->pfnSPIProbe(pSpiDevice, pDevice, &Error);
		if(Error) {
			BT_kFree(pSpiDevice);
			continue;
		}

		bt_list_add(&pSpiDevice->item, &master->pMaster->spidevices);
	}

#ifdef BT_CONFIG_OF
	struct bt_device_node *dev = bt_of_integrated_get_node(master->pMaster->pDevice);	// Node of bus device.
	if(!dev) {
		goto out;
	}

	struct bt_list_head *pos;
	bt_list_for_each(pos, &dev->children) {
		struct bt_device_node *spi_device = (struct bt_device_node *) pos;
		bt_of_spi_populate_device(spi_device);
		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(spi_device->dev.name);
		if(!pDriver || (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) != BT_DRIVER_SPI) {
			continue;
		}

		BT_SPI_DEVICE *pSpiDevice = BT_kMalloc(sizeof(*pSpiDevice));
		pSpiDevice->pMaster = master->pMaster;

		pDriver->pfnSPIProbe(pSpiDevice, &spi_device->dev, &Error);
		if(Error) {
			continue;
		}

		bt_list_add(&pSpiDevice->item, &master->pMaster->spidevices);
	}
#endif

out:
	BT_kPrint("All spi devices on bus probed");
}

static void spi_sm(void *pData) {
	struct spi_bus_item *master;

	bt_list_for_each_entry(master, &g_spi_masters, bus_list) {
		if(master->sm_flags & SM_FLAG_PROBE) {
			spi_probe_devices(master);
			master->sm_flags &= ~SM_FLAG_PROBE;
		}
	}
}

static BT_TASKLET spi_sm_tasklet = {NULL, BT_TASKLET_IDLE, spi_sm, NULL};

BT_ERROR BT_SpiRegisterMaster(BT_HANDLE hMaster, BT_SPI_MASTER *pMaster) {
  struct spi_bus_item *master;

  // Check if bus id already exists
  bt_list_for_each_entry(master, &g_spi_masters, bus_list){
    if( master->pMaster->bus_num == pMaster->bus_num)
      return BT_ERR_INVALID_RESOURCE;
  }

  master = BT_kMalloc(sizeof(*master));
  if(!master) {
	return BT_ERR_NO_MEMORY;
  }

  BT_LIST_INIT_HEAD(&pMaster->spidevices);
  master->hMaster = hMaster;
  master->pMaster = pMaster;
  master->sm_flags = SM_FLAG_PROBE;
  master->bus_mutex = BT_kMutexCreate();
  //bt_spin_lock_init(&master->bus_spinlock);

  master->pMaster->bus_item = master;

  bt_list_add(&master->bus_list, &g_spi_masters);

  BT_TaskletSchedule(&spi_sm_tasklet);

  return BT_ERR_NONE;
}

void BT_SpiMessageInit(BT_SPI_MESSAGE *pMessage)
{
	memset(pMessage, 0, sizeof *pMessage);
	BT_LIST_INIT_HEAD(&pMessage->transfers);
}

void BT_SpiMessageAddTail(BT_SPI_TRANSFER *pTransfer, BT_SPI_MESSAGE *pMessage)
{
	bt_list_add_tail(&pTransfer->transfer_list, &pMessage->transfers);
}

void BT_SpiTransferDel(BT_SPI_TRANSFER *pTransfer)
{
	bt_list_del(&pTransfer->transfer_list);
}

BT_ERROR BT_SpiSetup(BT_SPI_DEVICE *pDevice)
{
	BT_u16 bad_bits;
	BT_ERROR status = 0;

	/* help drivers fail *cleanly* when they need options
	 * that aren't supported with their current master
	 */

	bad_bits = pDevice->mode & ~pDevice->pMaster->mode_bits;
	if(bad_bits) {
		BT_kPrint("BT_SpiSetup: unsupported mode bits %x\n", bad_bits);
		return BT_ERR_INVALID_VALUE;
	}

	if(!pDevice->bits_per_word)
		pDevice->bits_per_word = 8;

	status = BT_IF_SPI_OPS(pDevice->pMaster->bus_item->hMaster)->pfnSetup(pDevice->pMaster->bus_item->hMaster, pDevice);

	BT_kPrint("BT_SpiSetup: setup mode %d, %s%s%s%s"
				"%u bits/w, %u Hz max --> %d",
				(BT_u32)(pDevice->mode & (SPI_CPOL | SPI_CPHA)),
				(pDevice->mode & SPI_CS_HIGH) ? "cs_high, " : "",
				(pDevice->mode & SPI_LSB_FIRST) ? "lsb, " : "",
				(pDevice->mode & SPI_3WIRE) ? "3wire, " : "",
				(pDevice->mode & SPI_LOOP) ? "loopback, " : "",
				pDevice->bits_per_word, pDevice->max_speed_hz,
				status);
	return status;
}


void BT_SpiComplete(void *arg)
{
	*(BT_BOOL*)arg = BT_TRUE;
}


BT_ERROR __BT_SpiAsync(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage)
{
	BT_SPI_MASTER *master = pDevice->pMaster;
	BT_SPI_TRANSFER *xfer;

	/* Half-duplex links include original MicroWire, and ones with
	 * only one data pin like SPI_3WIRE (switches direction) or where
	 * either MOSI or MISO is missing.  They can also be caused by
	 * software limitations.
	 */
	if((master->flags & SPI_MASTER_HALF_DUPLEX) || (pDevice->mode & SPI_3WIRE)) {
		BT_u16 flags = master->flags;

		bt_list_for_each_entry(xfer, &pMessage->transfers, transfer_list) {
			if(xfer->rx_buf && xfer->tx_buf)
				return BT_ERR_INVALID_VALUE;
			if((flags & SPI_MASTER_NO_TX) && xfer->tx_buf)
				return BT_ERR_INVALID_VALUE;
			if((flags & SPI_MASTER_NO_RX) && xfer->rx_buf)
				return BT_ERR_INVALID_VALUE;
		}
	}

	/**
	 * Set transfer bits_per_word and max speed as spi device default if
	 * it is not set for this transfer.
	 */
	bt_list_for_each_entry(xfer, &pMessage->transfers, transfer_list) {
		if(!xfer->bits_per_word)
			xfer->bits_per_word = pDevice->bits_per_word;
		if(!xfer->speed_hz)
			xfer->speed_hz = pDevice->max_speed_hz;
	}

	pMessage->spi_device = pDevice;
	pMessage->status = 1;

	return BT_IF_SPI_OPS(pDevice->pMaster->bus_item->hMaster)->pfnTransfer(pDevice->pMaster->bus_item->hMaster, pMessage);
}


BT_ERROR BT_SpiAsync(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage)
{
	BT_ERROR ret;

	//bt_spin_lock_irqsave(&pDevice->pMaster->bus_item->bus_spinlock,flags);

	//if(pDevice->pMaster->bus_lock_flag)
	//	ret = BT_ERR_BUSY;
	//else
		ret = __BT_SpiAsync(pDevice, pMessage);

	//bt_spin_unlock_irqrestore(&pDevice->pMaster->bus_item->bus_spinlock, flags);
	return ret;
}


BT_ERROR BT_SpiAsync_locked(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage)
{
	BT_ERROR ret;
	//bt_spin_lock_irqsave(&pDevice->pMaster->bus_item->bus_spinlock,flags);

	ret = __BT_SpiAsync(pDevice, pMessage);

	//bt_spin_unlock_irqrestore(&pDevice->pMaster->bus_item->bus_spinlock, flags);

	return ret;
}


BT_ERROR __BT_SpiSync(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage, BT_u32 bus_locked)
{
	BT_BOOL done = BT_FALSE;
	BT_ERROR status;

	pMessage->complete=BT_SpiComplete;
	pMessage->context = &done;

	if (!bus_locked)
		BT_kMutexPend(pDevice->pMaster->bus_item->bus_mutex, BT_INFINITE_TIMEOUT);

	status = BT_SpiAsync_locked(pDevice, pMessage);

	if(!bus_locked)
		BT_kMutexRelease(pDevice->pMaster->bus_item->bus_mutex);

	if(status == BT_ERR_NONE)
	{
		while(done != BT_TRUE) {
			BT_ThreadSleep(1);
		}
		status = pMessage->status;
	}
	pMessage->context = NULL;
	return status;
}

BT_ERROR BT_SpiSync(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage)
{
	return __BT_SpiSync(pDevice, pMessage, 0);
}

BT_ERROR BT_SpiBusLock(BT_SPI_MASTER *pMaster)
{
	if(pMaster && pMaster->bus_item->bus_mutex) {
		BT_kMutexPend(pMaster->bus_item->bus_mutex, BT_INFINITE_TIMEOUT);
		return BT_ERR_NONE;
	}
	return BT_ERR_INVALID_HANDLE;
}

BT_ERROR BT_SpiBusUnlock(BT_SPI_MASTER *pMaster)
{
	if(pMaster && pMaster->bus_item->bus_mutex) {
		BT_kMutexRelease(pMaster->bus_item->bus_mutex);
		return BT_ERR_NONE;
	}
	return BT_ERR_INVALID_HANDLE;
}

BT_ERROR BT_SpiWrite(BT_SPI_DEVICE *pDevice, const void *buf, BT_u32 len)
{
	BT_SPI_TRANSFER t = {
			.tx_buf	= buf,
			.len	= len,
	};

	BT_SPI_MESSAGE m;
	BT_SpiMessageInit(&m);
	BT_SpiMessageAddTail(&t,&m);
	return BT_SpiSync(pDevice, &m);
}

BT_ERROR BT_SpiRead(BT_SPI_DEVICE *pDevice, void *buf, BT_u32 len)
{
	BT_SPI_TRANSFER t = {
			.rx_buf	= buf,
			.len	= len,
	};

	BT_SPI_MESSAGE m;
	BT_SpiMessageInit(&m);
	BT_SpiMessageAddTail(&t, &m);
	return BT_SpiSync(pDevice, &m);
}

BT_ERROR BT_SpiWriteThenRead(BT_SPI_DEVICE *pDevice, const void *txbuf, BT_u32 n_tx, void *rxbuf, BT_u32 n_rx)
{
	BT_ERROR status;
	BT_SPI_MESSAGE message;
	BT_SPI_TRANSFER x[2];
	BT_u8 *local_buf;

	local_buf = BT_kMalloc(n_tx + n_rx);
	if(!local_buf)
		return BT_ERR_NO_MEMORY;

	BT_SpiMessageInit(&message);
	memset(x, 0, sizeof x);
	if(n_tx) {
		x[0].len = n_tx;
		BT_SpiMessageAddTail(&x[0], &message);
	}
	if(n_rx) {
		x[1].len = n_rx;
		BT_SpiMessageAddTail(&x[1], &message);
	}

	memcpy(local_buf, txbuf, n_tx);
	x[0].tx_buf = local_buf;
	x[1].rx_buf = local_buf + n_tx;

	/* do the i/o */
	status = BT_SpiSync(pDevice,&message);
	if(status == BT_ERR_NONE)
		memcpy(rxbuf,x[1].rx_buf, n_rx);

	BT_kFree(local_buf);

	return status;
}
