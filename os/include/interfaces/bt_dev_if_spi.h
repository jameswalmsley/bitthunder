#ifndef _BT_DEV_IF_SPI_H_
#define _BT_DEV_IF_SPI_H_

#include "bt_types.h"
#include <collections/bt_list.h>
#include <helpers/bt_bitops.h>

struct spi_bus_item;

#define BT_SPI_NAME_SIZE		32
#define BT_SPI_MODULE_PREFIX	"spi:"

struct bt_spi_device_id {
	char name[BT_SPI_NAME_SIZE];
	BT_u32 driver_data;
};

typedef struct _BT_SPI_MASTER {
	struct bt_list_head	spidevices;
	const BT_DEVICE    *pDevice;
	BT_u16 				bus_num;
	BT_u16				num_chipselect;
	BT_u16				dma_alignment;
	BT_u16				mode_bits;
	BT_u16				flags;
#define SPI_MASTER_HALF_DUPLEX	BT_BIT(0)		/* can't do full duplex */
#define SPI_MASTER_NO_RX		BT_BIT(1)		/* can't do buffer read */
#define SPI_MASTER_NO_TX		BT_BIT(2)		/* can't do buffer write */
#define SPI_MASTER_U_PAGE		BT_BIT(3)		/* select upper flash */
#define SPI_MASTER_QUAD_MODE	BT_BIT(4)		/* support quad mode */
	struct spi_bus_item *bus_item; 				//< internal data
} BT_SPI_MASTER;

typedef struct _BT_SPI_DEVICE {
	struct bt_list_head	item;
	BT_SPI_MASTER 	*pMaster;
	BT_u32			max_speed_hz;
	BT_u8			chip_select;
	BT_u8			mode;
#define SPI_CPHA        0x01                    /* clock phase */
#define SPI_CPOL        0x02                    /* clock polarity */
#define SPI_MODE_0      (0|0)                   /* (original MicroWire) */
#define SPI_MODE_1      (0|SPI_CPHA)
#define SPI_MODE_2      (SPI_CPOL|0)
#define SPI_MODE_3      (SPI_CPOL|SPI_CPHA)
#define SPI_CS_HIGH     0x04                    /* chipselect active high? */
#define SPI_LSB_FIRST   0x08                    /* per-word bits-on-wire */
#define SPI_3WIRE       0x10                    /* SI/SO signals shared */
#define SPI_LOOP        0x20                    /* loopback mode */
#define SPI_NO_CS       0x40                    /* 1 dev/bus, no chipselect */
#define SPI_READY       0x80                    /* slave pulls low to pause */
	BT_u8			bits_per_word;
} BT_SPI_DEVICE;

typedef struct _BT_SPI_TRANSFER {
	struct bt_list_head transfer_list;

    const void      *tx_buf;
    void            *rx_buf;
    BT_u32        	 len;

    BT_u32	         cs_change:1;
    BT_u8            bits_per_word;
    BT_u16           delay_usecs;
    BT_u32           speed_hz;
} BT_SPI_TRANSFER;

typedef struct _BT_SPI_MESSAGE {
    struct bt_list_head			transfers;
    BT_SPI_DEVICE			   *spi_device;
    BT_u32						status;
    BT_u32					    actual_length;
    void 					   	(*complete)(void *context);
    void					   *context;
} BT_SPI_MESSAGE;

typedef struct {
	BT_ERROR	(*pfnSetup)		(BT_HANDLE hMaster, BT_SPI_DEVICE *pDevice);
	BT_u32	 	(*pfnTransfer) 	(BT_HANDLE hMaster, BT_SPI_MESSAGE *message);
} BT_DEV_IF_SPI;

/*
 *	Define the unified API for SPI devices in BitThunder
 */
BT_ERROR BT_SpiRegisterMaster(BT_HANDLE hMaster, BT_SPI_MASTER *pMaster);
void BT_SpiMessageInit(BT_SPI_MESSAGE *pMessage);
void BT_SpiMessageAddTail(BT_SPI_TRANSFER *pTransfer, BT_SPI_MESSAGE *pMessage);
void BT_SpiTransferDel(BT_SPI_TRANSFER *pTransfer);
BT_ERROR BT_SpiSetup(BT_SPI_DEVICE *pDevice);
BT_ERROR BT_SpiSync(BT_SPI_DEVICE *pDevice, BT_SPI_MESSAGE *pMessage);
BT_ERROR BT_SpiBusLock(BT_SPI_MASTER *pMaster);
BT_ERROR BT_SpiBusUnlock(BT_SPI_MASTER *pMaster);

BT_ERROR BT_SpiWrite(BT_SPI_DEVICE *pDevice, const void *buf, BT_u32 len);
BT_ERROR BT_SpiRead(BT_SPI_DEVICE *pDevice, void *buf, BT_u32 len);
BT_ERROR BT_SpiWriteThenRead(BT_SPI_DEVICE *pDevice, const void *txbuf, BT_u32 n_tx, void *rxbuf, BT_u32 n_rx);

#endif
