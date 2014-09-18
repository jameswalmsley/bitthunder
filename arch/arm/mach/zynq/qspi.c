#include <bitthunder.h>
#include <collections/bt_list.h>
#include <of/bt_of.h>
#include <string.h>
#include "qspi.h"
#include "slcr.h"

BT_DEF_MODULE_NAME				("Zynq-QSPI")
BT_DEF_MODULE_DESCRIPTION		("QSPI device driver for Zynq")
BT_DEF_MODULE_AUTHOR			("Michael Daniel")
BT_DEF_MODULE_EMAIL				("mdaniel@riegl.com")

/*
 * The modebits configurable by the driver to make the SPI support different
 * data formats
 */
#define MODEBITS                        (SPI_CPOL | SPI_CPHA)

/*
 * Definitions for the status of queue
 */
#define QSPI_USE_WORKQUEUE			 0
#define QSPI_QUEUE_STOPPED           0
#define QSPI_QUEUE_RUNNING           1

/*
 * Definitions of the flash commands
 */
/* Flash opcodes in ascending order */
#define QSPI_FLASH_OPCODE_WRSR       0x01    /* Write status register */
#define QSPI_FLASH_OPCODE_PP         0x02    /* Page program */
#define QSPI_FLASH_OPCODE_NORM_READ  0x03    /* Normal read data bytes */
#define QSPI_FLASH_OPCODE_WRDS       0x04    /* Write disable */
#define QSPI_FLASH_OPCODE_RDSR1      0x05    /* Read status register 1 */
#define QSPI_FLASH_OPCODE_WREN       0x06    /* Write enable */
#define QSPI_FLASH_OPCODE_BRRD       0x16    /* Bank Register Read */
#define QSPI_FLASH_OPCODE_BRWR       0x17    /* Bank Register Write */
#define QSPI_FLASH_OPCODE_EXTADRD    0xC8    /* Micron - Bank Reg Read */
#define QSPI_FLASH_OPCODE_EXTADWR    0xC5    /* Micron - Bank Reg Write */
#define QSPI_FLASH_OPCODE_FAST_READ  0x0B    /* Fast read data bytes */
#define QSPI_FLASH_OPCODE_BE_4K      0x20    /* Erase 4KiB block */
#define QSPI_FLASH_OPCODE_RDSR2      0x35    /* Read status register 2 */
#define QSPI_FLASH_OPCODE_RDFSR      0x70    /* Read flag status register */
#define QSPI_FLASH_OPCODE_DUAL_READ  0x3B    /* Dual read data bytes */
#define QSPI_FLASH_OPCODE_BE_32K     0x52    /* Erase 32KiB block */
#define QSPI_FLASH_OPCODE_QUAD_READ  0x6B    /* Quad read data bytes */
#define QSPI_FLASH_OPCODE_ERASE_SUS  0x75    /* Erase suspend */
#define QSPI_FLASH_OPCODE_ERASE_RES  0x7A    /* Erase resume */
#define QSPI_FLASH_OPCODE_RDID       0x9F    /* Read JEDEC ID */
#define QSPI_FLASH_OPCODE_BE         0xC7    /* Erase whole flash block */
#define QSPI_FLASH_OPCODE_SE         0xD8    /* Sector erase (usually 64KB)*/
#define QSPI_FLASH_OPCODE_QPP		 0x32	 /* Quad page program */


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				 	 h;
	volatile ZYNQ_QSPI_REGS 			*pRegs;
	volatile ZYNQ_SLCR_REGS 			*pSLCR;

	BT_SPI_MASTER						spi_master;

#if (QSPI_USE_WORKQUEUE)
	struct workqueue_struct 			*workqueue;
	struct work_struct					work;
	struct bt_list_head					queue;
	BT_u8								queue_state;
#endif
	BT_u8								dev_busy;

	volatile BT_i32						bytes_to_transfer;
	volatile BT_i32						bytes_to_receive;
	const void 			   			   *txbuf;
	void 					   		   *rxbuf;

	volatile BT_u32 								speed_hz;

	BT_BOOL 							is_dual;
	volatile BT_BOOL 					done;
	BT_BOOL 							is_inst;

	volatile BT_u32								irq;
};

/**
 * struct xqspips_inst_format - Defines qspi flash instruction format
 * @opcode:             Operational code of instruction
 * @inst_size:          Size of the instruction including address bytes
 **/
struct qspi_inst_format {
	volatile BT_u8 opcode;
	volatile BT_u8 inst_size;
};

/*
 * List of all the QSPI instructions and its format
 */
static struct qspi_inst_format flash_inst[] = {
        { QSPI_FLASH_OPCODE_WREN, 1 },
        { QSPI_FLASH_OPCODE_WRDS, 1 },
        { QSPI_FLASH_OPCODE_RDSR1, 1 },
        { QSPI_FLASH_OPCODE_RDSR2, 1 },
        { QSPI_FLASH_OPCODE_WRSR, 1},
        { QSPI_FLASH_OPCODE_RDFSR, 1 },
        { QSPI_FLASH_OPCODE_PP, 4 },
        { QSPI_FLASH_OPCODE_SE, 4 },
        { QSPI_FLASH_OPCODE_BE_32K, 4 },
        { QSPI_FLASH_OPCODE_BE_4K, 4 },
        { QSPI_FLASH_OPCODE_BE, 1 },
        { QSPI_FLASH_OPCODE_ERASE_SUS, 1 },
        { QSPI_FLASH_OPCODE_ERASE_RES, 1 },
        { QSPI_FLASH_OPCODE_RDID, 1 },
        { QSPI_FLASH_OPCODE_NORM_READ, 4 },
        { QSPI_FLASH_OPCODE_FAST_READ, 1 },
        { QSPI_FLASH_OPCODE_DUAL_READ, 1 },
        { QSPI_FLASH_OPCODE_QUAD_READ, 1 },
        { QSPI_FLASH_OPCODE_BRRD, 1 },
        { QSPI_FLASH_OPCODE_BRWR, 2 },
        { QSPI_FLASH_OPCODE_EXTADRD, 1 },
        { QSPI_FLASH_OPCODE_EXTADWR, 2 },
        { QSPI_FLASH_OPCODE_QPP, 4 },
        /* Add all the instructions supported by the flash device */
};


static void qspi_init_hw(BT_HANDLE qspi, int is_dual)
{
	BT_u32 config_reg;
	qspi->pRegs->ENABLE = ~QSPI_ENABLE_ENABLE_MASK;
	qspi->pRegs->INT_DISABLE = 0x7F;

	/* Disable linear mode as the boot loader may have used it */
	qspi->pRegs->LINEAR_CFG = 0;

	/* Clear the RX FIFO */
	while(qspi->pRegs->INT_STATUS & QSPI_IXR_RXNEMTY_MASK)
		qspi->pRegs->RXD = qspi->pRegs->RXD;

	qspi->pRegs->INT_STATUS = 0x7F;
	config_reg = qspi->pRegs->CONFIG;
	config_reg &= 0xFBFFFFFF; /* Set little endian mode of TX FIFO */
	config_reg |= 0x8000FCC1;
	qspi->pRegs->CONFIG = config_reg;


	if(is_dual == 1)
		/* Enable two memories on seperate buses */
		qspi->pRegs->LINEAR_CFG = (QSPI_LCFG_TWO_MEM_MASK | QSPI_LCFG_SEP_BUS_MASK | (1 << QSPI_LCFG_DUMMY_SHIFT) | QSPI_FAST_READ_QOUT_CODE);

	#ifdef CONFIG_SPI_XILINX_PS_QSPI_DUAL_STACKED
	qspi->pLinearCfgReg->LINEAR_CFG = (QSPI_LCFG_TWO_MEM_MASK | (1 << QSPI_LCFG_DUMMY_SHIFT) | QSPI_FAST_READ_QOUT_CODE);
	#endif

	qspi->pRegs->ENABLE = QSPI_ENABLE_ENABLE_MASK;

}

static void qspi_copy_read_data(BT_HANDLE qspi, BT_u32 data, BT_u8 size)
{
	if(qspi->rxbuf)	{
		data >>= (4 - size) * 8;
		data = bt_le32_to_cpu(data);
		memcpy((BT_u8*)qspi->rxbuf, &data, size);
		qspi->rxbuf += size;
	}
	qspi->bytes_to_receive -= size;
	if(qspi->bytes_to_receive < 0) {
		qspi->bytes_to_receive = 0;
	}
}

static void qspi_copy_write_data(BT_HANDLE qspi, BT_u32 *data, BT_u8 size)
{
	if(qspi->txbuf) {
		switch(size) {
			case 1:
				*data = *((BT_u8 *)qspi->txbuf);
				qspi->txbuf += 1;
				*data |= 0xFFFFFF00;
				break;
			case 2:
				*data = *((BT_u16 *)qspi->txbuf);
				qspi->txbuf += 2;
				*data |= 0xFFFF0000;
				break;
			case 3:
				*data = *((BT_u16 *)qspi->txbuf);
				qspi->txbuf += 2;
				*data |= (*((BT_u8 *) qspi->txbuf) << 16);
				qspi->txbuf += 1;
				*data |= 0xFF000000;
				break;
			case 4:
				*data = *((BT_u32 *)qspi->txbuf);
				qspi->txbuf += 4;
				break;
			default:
				/* This will never execute */
				break;
		}
	} else {
		*data = 0;
	}

	qspi->bytes_to_transfer -= size;
	if(qspi->bytes_to_transfer < 0) {
		qspi->bytes_to_transfer = 0;
	}
}


static void qspi_chipselect(BT_HANDLE qspi, BT_SPI_DEVICE *pDevice, int is_on)
{
	BT_u32 config_reg;

	BT_DisableInterrupt(qspi->irq);
	{
		config_reg = qspi->pRegs->CONFIG;

		if(is_on) {
			/* select the slave */
			config_reg &= ~QSPI_CONFIG_SSCTRL_MASK;
			config_reg |= (((~(0x0001 << pDevice->chip_select)) << 10) & QSPI_CONFIG_SSCTRL_MASK);
		} else {
			/* Deselect the slave */
			config_reg |= QSPI_CONFIG_SSCTRL_MASK;
		}

		qspi->pRegs->CONFIG = config_reg;
	}
	BT_EnableInterrupt(qspi->irq);
}

/*__attribute__((optimize("-O0")))*/ static BT_ERROR qspi_setup_transfer(BT_HANDLE qspi, BT_SPI_DEVICE *pDevice, BT_SPI_TRANSFER *transfer)
{
	BT_u32 config_reg;
	BT_u32 req_hz;
	BT_u32 baud_rate_val = 0;
	BT_BOOL update_baud = BT_FALSE;

	req_hz = (transfer) ? transfer->speed_hz : pDevice->max_speed_hz;

	if(pDevice->mode & ~MODEBITS) {
		BT_kPrint("qspi: unsupported mode bits %x\n",pDevice->mode & ~MODEBITS);
		return BT_ERR_UNSUPPORTED_FLAG;
	}

	if(transfer && (transfer->speed_hz == 0)) {
		req_hz = pDevice->max_speed_hz;
	}

	/* Set the clock frequency */
	if(qspi->speed_hz != req_hz)
	{

		BT_u32 clk_sel = ZYNQ_SLCR_CLK_CTRL_SRCSEL_VAL(qspi->pSLCR->LQSPI_CLK_CTRL);
		BT_u32 InputClk;
		switch(clk_sel) {
		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_ARMPLL:
			InputClk = BT_ZYNQ_GetArmPLLFrequency();
			break;
		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_IOPLL:
			InputClk = BT_ZYNQ_GetIOPLLFrequency();
			break;

		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_DDRPLL:
			InputClk = BT_ZYNQ_GetDDRPLLFrequency();
			break;

		default:
			return -1;
		}

		InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(qspi->pSLCR->LQSPI_CLK_CTRL);

		while((baud_rate_val < 8) && (InputClk / (2 << baud_rate_val)) > req_hz ) {
			baud_rate_val++;
		}

		qspi->speed_hz = req_hz;
		update_baud = BT_TRUE;

		//BT_kPrint("QSPI setup-transfer: Current Frequenzy: %d", InputClk / (2 << baud_rate_val));
	}

	//BT_DisableInterrupt(qspi->irq);
	BT_kEnterCritical();
	{
		config_reg = qspi->pRegs->CONFIG;

		/* Set the QSPI clock phase and clock polarity */
		config_reg &= (~QSPI_CONFIG_CPHA_MASK) & (~QSPI_CONFIG_CPOL_MASK);
		if (pDevice->mode & SPI_CPHA)
			config_reg |= QSPI_CONFIG_CPHA_MASK;
		if (pDevice->mode & SPI_CPOL)
			config_reg |= QSPI_CONFIG_CPOL_MASK;

		if (update_baud) {
			config_reg &= 0xFFFFFFC7;
			config_reg |= (baud_rate_val << 3);
		}

		qspi->pRegs->CONFIG = config_reg;
	}
	BT_kExitCritical();
	//BT_EnableInterrupt(qspi->irq);

	return BT_ERR_NONE;
}

static BT_ERROR qspi_setup(BT_HANDLE qspi, BT_SPI_DEVICE *pDevice)
{
	if (pDevice->mode & SPI_LSB_FIRST)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->max_speed_hz)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->bits_per_word)
		pDevice->bits_per_word = 32;

	return qspi_setup_transfer(qspi, pDevice, NULL);
}

static void qspi_fill_tx_fifo(BT_HANDLE qspi)
{
	BT_u32 data = 0;

	while((!(qspi->pRegs->INT_STATUS & QSPI_IXR_TXFULL_MASK)) && (qspi->bytes_to_transfer >= 4)) {
		qspi_copy_write_data(qspi,&data,4);
		qspi->pRegs->TXD_00_00 = data;
	}
}

static BT_ERROR qspi_irq(BT_u32 ulIRQ, void *pParam)
{
	BT_HANDLE qspi = (BT_HANDLE) pParam;
	BT_u32 intr_status;

	intr_status = qspi->pRegs->INT_STATUS;
	qspi->pRegs->INT_STATUS = intr_status;
	qspi->pRegs->INT_DISABLE = QSPI_IXR_ALL_MASK;

	if ((intr_status & QSPI_IXR_TXNFULL_MASK) || (intr_status & QSPI_IXR_RXNEMTY_MASK )) {
		/* This bit is set when TX FIFO has < THRESHOLD entries. We have the THRESHOLD value
		 * set to 1, so this bit indicates Tx FIFO is empty
		 */
		BT_u32 config_reg;
		BT_u32 data;

		/* Read out the data from the RX FIFO */
		while (qspi->pRegs->INT_STATUS & QSPI_IXR_RXNEMTY_MASK) {
			data = qspi->pRegs->RXD;
			if (qspi->bytes_to_receive < 4 && !qspi->is_dual) {
				qspi_copy_read_data(qspi, data, qspi->bytes_to_receive);
			}
			else {
				qspi_copy_read_data(qspi, data, 4);
			}
		}

		if (qspi->bytes_to_transfer) {
			if (qspi->bytes_to_transfer >= 4) {
				/* There is more data to send */
				qspi_fill_tx_fifo(qspi);
			}
			else {
				int tmp = qspi->bytes_to_transfer;
				qspi_copy_write_data(qspi, &data, qspi->bytes_to_transfer);
				if (qspi->is_dual) {
					qspi->pRegs->TXD_00_00 = data;
				}
				else {
					switch(tmp)
					{
						case 1:
							qspi->pRegs->TXD_00_01 = data;
							break;
						case 2:
							qspi->pRegs->TXD_00_10 = data;
							break;
						case 3:
							qspi->pRegs->TXD_00_11 = data;
							break;
						default:
							break;
					}
				}
			}
			qspi->pRegs->INT_ENABLE = QSPI_IXR_ALL_MASK;

			// do not use mutex inside an ISR!!

			// spin_lock(qspi->config_reg_lock);
			//BT_kMutexPend(qspi->pMutexConfigReg, BT_INFINITE_TIMEOUT);
			config_reg = qspi->pRegs->CONFIG;
			config_reg |= QSPI_CONFIG_MANSRT_MASK;
			qspi->pRegs->CONFIG = config_reg;
			//BT_kMutexRelease(qspi->pMutexConfigReg);
			// spin_unlock(qspi->config_reg_lock);
		}
		else {
			/* If transfer and receive is completed then only send
			 * complete signal.
			 */
			if (qspi->bytes_to_receive) {
				/* There is still some data to be received.
				 * Enable Rx not empty interrupt.
				 */
				qspi->pRegs->INT_ENABLE = QSPI_IXR_RXNEMTY_MASK;
			}
			else {
				qspi->pRegs->INT_DISABLE = QSPI_IXR_RXNEMTY_MASK;
				// TODO: maybe use some kind of completion struct
				qspi->done = BT_TRUE;
			}
		}
	}

	return BT_ERR_NONE;
}

static BT_ERROR qspi_cleanup(BT_HANDLE hQspi)
{
	hQspi->pRegs->ENABLE = ~QSPI_ENABLE_ENABLE_MASK;

	BT_UnregisterInterrupt(hQspi->irq, qspi_irq, hQspi);

	bt_iounmap(hQspi->pRegs);
	bt_iounmap(hQspi->pSLCR);

	BT_DestroyHandle(hQspi);

	return BT_ERR_NONE;
}

static BT_i32 qspi_start_transfer(BT_HANDLE qspi, BT_SPI_TRANSFER *transfer)
{
	BT_u32 data;
	BT_u8 instruction = 0;
	BT_u8 index;
	struct qspi_inst_format *curr_inst;

	qspi->txbuf = transfer->tx_buf;
	qspi->rxbuf = transfer->rx_buf;
	qspi->bytes_to_transfer = transfer->len;
	qspi->bytes_to_receive = transfer->len;

	if (qspi->txbuf)
		instruction = *(BT_u8 *)qspi->txbuf;


	//INIT_COMPLETION(qspi->done);
	qspi->done = BT_FALSE;

	if(instruction && qspi->is_inst) {
		for(index = 0; index < BT_ARRAY_SIZE(flash_inst); index++ )
			if (instruction == flash_inst[index].opcode)
				break;

		/* Instruction might have already been transmitted. This is a
		 * 'data only' transfer
		 */
		if(index == BT_ARRAY_SIZE(flash_inst))
			goto xfer_data;

		curr_inst = &flash_inst[index];

		/* Get the instruction */
		data = 0;
		qspi_copy_write_data(qspi, &data, curr_inst->inst_size);

		/* Write the instruction to LSB of the FIFO. The core is
		 * designed such that it is not necessary to check wether the
		 * write FIFO is full before writing. However, write would be
		 * delayed if the user tries to write when write FIFO is full
		 */
		switch(curr_inst->inst_size) {
			case 1:
				qspi->pRegs->TXD_00_01 = data;
				break;
			case 2:
				qspi->pRegs->TXD_00_10 = data;
				break;
			case 3:
				qspi->pRegs->TXD_00_11 = data;
				break;
			case 4:
				qspi->pRegs->TXD_00_00 = data;
				break;
			default:
				break;
		}
		goto xfer_start;
	}

xfer_data:
	/* In case of Fast, Dual and Quad reads, transmit the instruction first.
	 * Address and dummy byte will be transmitted in interrupt handler,
	 * after instruction is transmitted
	 */
	if (((qspi->is_inst == 0) && (qspi->bytes_to_transfer >= 4)) ||
			((qspi->bytes_to_transfer >= 4) &&
			(instruction != QSPI_FLASH_OPCODE_FAST_READ) &&
			(instruction != QSPI_FLASH_OPCODE_DUAL_READ) &&
			(instruction != QSPI_FLASH_OPCODE_QUAD_READ)))
		qspi_fill_tx_fifo(qspi);

xfer_start:
	qspi->pRegs->INT_ENABLE = QSPI_IXR_ALL_MASK;

	/* start the transfer by enabling manual start bit */
	//BT_DisableInterrupt(qspi->irq);
	BT_kEnterCritical();
	{
		qspi->pRegs->CONFIG |= QSPI_CONFIG_MANSRT_MASK;
	}
	BT_kExitCritical();
	//BT_EnableInterrupt(qspi->irq);

	// wait_for_completion(qspi->done);
	while(!qspi->done) {}
	return (transfer->len) - (qspi->bytes_to_transfer);
}

//#if	(QSPI_USE_WORKQUEUE)
//static void qspi_work_queue(BT_HANDLE hQspi, struct bt_work_struct * work)
//{
//#ifdef CONFIG_SPI_XILINX_PS_QSPI_DUAL_STACKED
// 	BT_u32 lqspi_cfg_reg;
//#endif
//
// 	BT_DisableInterrupt(hQspi->irq);
// 	BT_kMutexPend(hQspi->pMutexTransfer, BT_INFINITE_TIMEOUT);
// 	hQspi->dev_busy = 1;
//
// 	/* Check if list is empty or queue is stopped */
// 	if (bt_list_empty(hQspi->queue) || hQspi->queue_state == QSPI_QUEUE_STOPPED) {
// 		hQspi->dev_busy = 0;
// 		BT_kMutexRelease(hQspi->pMutexTransfer);
// 		BT_EnableInterrupt(hQspi->irq);
// 		return;
// 	}
//
// 	/* Keep requesting transfer till list is empty */
// 	while(!bt_list_empty(hQspi->queue)) {
// 		BT_SPI_MESSAGE 		*msg;
// 		BT_SPI_DEVICE 		*qspi_dev;
// 		BT_SPI_TRANSFER		*transfer = NULL;
// 		unsigned			cs_change = 1;
// 		int				status = 0;
//
// 		msg = bt_container_of(hQspi->queue.next, BT_SPI_MESSAGE, queue);
// 		bt_list_del_init(&msg->queue);
// 		BT_kMutexRelease(hQspi->pMutexTransfer);
// 		BT_EnableInterrupt(hQspi->irq);
// 		qspi_dev = msg->spi;
//
//#ifdef CONFIG_SPI_XILINX_PS_QSPI_DUAL_STACKED
// 		lqspi_cfg_reg = hQspi->pRegs->LINEAR_CFG;
// 		if (hQspi->spi_master->flags & SPI_MASTER_U_PAGE)
// 			lqspi_cfg_reg |= QSPI_LCFG_U_PAGE_MASK;
// 		else {
// 			lqspi_cfg_reg &= ~QSPI_LCFG_U_PAGE_MASK;
// 			hQspi->pRegs->LINEAR_CFG = lqspi_cfg_reg;
// 		}
//#endif
//
// 		bt_list_for_each_entry(transfer, &msg->transfers, transfer_list) {
// 			if (transfer->bits_per_word || transfer->speed_hz) {
// 				status = qspi_setup_transfer(hQspi,transfer);
// 				if (status < 0)
// 					break;
// 			}
//
// 			/* Select the chip if required */
// 			if (cs_change) {
// 				qspi_chipselect(hQspi, 1);
// 				hQspi->is_inst = 1;
// 			}
//
// 			cs_change = transfer->cs_change;
//
// 			if(!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
// 				status = -BT_ERR_INVALID_HANDLE;
// 				break;
// 			}
//
// 			/* Request the transfer */
// 			if (transfer->len) {
// 				status = qspi_start_transfer(hQspi, transfer);
// 				hQspi->is_inst = 0;
// 			}
//
// 			if (status != transfer->len) {
// 				if (status > 0)
// 					status = -1;
// 				break;
// 			}
// 			msg->actual_length += status;
// 			status = 0;
//
// 			if (transfer->delay_usecs)
// 			{/* FIXME: udelay(transfer->delay_usecs); */ }
//
// 			if (cs_change)
// 				/* Deselect the chip */
// 				qspi_chipselect(hQspi, 0);
//
// 			if (transfer->transfer_list.next == &msg->transfers)
// 				break;
// 		}
//
// 		msg->status = status;
// 		msg->complete(msg->context);
//
// 		qspi_setup_transfer(hQspi, NULL);
//
// 		if(!(status == 0 && cs_change))
// 			qspi_chipselect(hQspi, 0);
//
// 		BT_DisableInterrupt(hQspi->irq);
// 		BT_kMutexPend(hQspi->pMutexTransfer);
// 	}
// 	hQspi->dev_busy = 0;
// 	BT_kMutexRelease(hQspi->pMutexTransfer);
// 	BT_EnableInterrupt(hQspi->irq);
//}
//#endif

BT_s32 qspi_transfer(BT_HANDLE hQspi, BT_SPI_MESSAGE * message) {

	BT_SPI_TRANSFER *transfer;
#if	(QSPI_USE_WORKQUEUE)
	if(qspi->queue_state == QSPI_QUEUE_STOPPED)
		return -BT_ERR_INVALID_RESOURCE;
#else
	int status = 0;
	unsigned			cs_change = 1;
#endif

	message->actual_length = 0;
	message->status = -1;

	/* Check each transfer's parameters */
	bt_list_for_each_entry(transfer, &message->transfers, transfer_list) {
		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len)
			return BT_ERR_INVALID_RESOURCE;
	}


#if	(QSPI_USE_WORKQUEUE)
	BT_DisableInterrupt(hQspi->irq);
	BT_kMutexPend(hQspi->pMutexTransfer, BT_INFINITE_TIMEOUT);
	bt_list_add_tail(&message->queue, &qspi->queue);
	if (!qspi->dev_busy)
		queue_work(qspi->workqueue, &qspi->work);
	BT_kMutexRelease(hQspi->pMutexTransfer);
	BT_EnableInterrupt(hQspi->irq);
#else
	hQspi->dev_busy = 1;

	bt_list_for_each_entry(transfer, &message->transfers, transfer_list) {
		if (transfer->bits_per_word || transfer->speed_hz) {
			status = qspi_setup_transfer(hQspi, message->spi_device, transfer);
			if (status != BT_ERR_NONE)
				break;
		}

		/* Select the chip if required */
		if (cs_change) {
			qspi_chipselect(hQspi, message->spi_device, 1);
			hQspi->is_inst = 1;
		}

		cs_change = transfer->cs_change;

		if(!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
			status = BT_ERR_INVALID_VALUE;
			break;
		}

		/* Request the transfer */
		if (transfer->len) {
			status = qspi_start_transfer(hQspi, transfer);
			hQspi->is_inst = 0;
		}

		if (status != transfer->len) {
			//if (status > 0)
			//	status = -1;
			break;
		}
		message->actual_length += status;
		status = 0;

		//if (transfer->delay_usecs)
		//{/* FIXME: udelay(transfer->delay_usecs); */ }

		if (cs_change)
			/* Deselect the chip */
			qspi_chipselect(hQspi, message->spi_device, 0);

		if (transfer->transfer_list.next == &message->transfers)
			break;
	}

	message->status = status;
	message->complete(message->context);

	qspi_setup_transfer(hQspi, message->spi_device, NULL);

	if(!(status == 0 && cs_change))
		qspi_chipselect(hQspi, message->spi_device, 0);

	hQspi->dev_busy = 0;
#endif

	return BT_ERR_NONE;
}

#if	(QSPI_USE_WORKQUEUE)
static inline int qspi_start_queue(BT_HANDLE hQspi)
{
	BT_DisableInterrupt(hQspi->irq);
	BT_kMutexPend(hQspi->pMutexTransfer, BT_INFINITE_TIMEOUT);

	if (hQspi->queue_state == QSPI_QUEUE_RUNNING || hQspi->dev_busy) {
		BT_kMutexRelease(hQspi->pMutexTransfer);
		BT_EnableInterrupt(hQspi->irq);
		return -1; // -EBUSY
	}

	hQspi->queue_state = QSPI_QUEUE_RUNNING;
	BT_kMutexRelease(hQspi->pMutexTransfer);
	BT_EnableInterrupt(hQspi->irq);

	return 0;
}


static inline int qspi_stop_queue(BT_HANDLE hQspi)
{
	unsigned limit = 500;
	int ret = 0;

	if (hQspi->queue_state != QSPI_QUEUE_RUNNING)
		return ret;

	BT_DisableInterrupt(hQspi->irq);
	BT_kMutexPend(hQspi->pMutexTransfer, BT_INFINITE_TIMEOUT);

	while ((!bt_list_empty(&hQspi->queue) || hQspi->dev_busy) && limit--) {
		BT_kMutexRelease(hQspi->pMutexTransfer);
		BT_EnableInterrupt(hQspi->irq);
		sys_msleep(10);
		BT_DisableInterrupt(hQspi->irq);
		BT_kMutexPend(hQspi->pMutexTransfer, BT_INFINITE_TIMEOUT);
	}

	if (!bt_list_empty(&hQspi->queue) || hQspi->dev_busy)
		ret = -1; //-EBUSY

	if (ret == 0)
		hQspi->queue_state = QSPI_QUEUE_STOPPED;

	BT_kMutexRelease(hQspi->pMutexTransfer);
	BT_EnableInterrupt(hQspi->irq);

	return ret;
}



static inline int qspi_destroy_queue(BT_HANDLE hQspi)
{
	int ret;

	ret = qspi_stop_queue(hQspi);
	if (ret != 0)
		return ret;

	destroy_workqueue(hQspi->workqueue);

	return 0;
}
#endif


static const BT_DEV_IF_SPI oSPIInterface = {
	.pfnTransfer		= qspi_transfer,
	.pfnSetup			= qspi_setup,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pPowerIF = NULL,
	.eConfigType = BT_DEV_IF_T_SPI,
	.unConfigIfs = {
		.pSpiIF = &oSPIInterface,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.eType = BT_HANDLE_T_DEVICE,
	.pfnCleanup = qspi_cleanup,
};

static BT_HANDLE qspi_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hQSPI = NULL;
	BT_u32 mem_start = 0;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_out;
	}

	hQSPI = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hQSPI) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hQSPI->pRegs = (ZYNQ_QSPI_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_1K);	//FIXME
	mem_start = pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	BT_u32 ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(ulIRQ, qspi_irq, hQSPI);
	if(Error) {
		Error = BT_ERR_GENERIC;	// Device already in use!
		goto err_free_out;
	}

	hQSPI->irq = ulIRQ;
	BT_EnableInterrupt(ulIRQ);

	hQSPI->spi_master.pDevice = pDevice;

#ifdef BT_CONFIG_OF
	struct bt_device_node *dev = bt_of_integrated_get_node(pDevice);
#endif

	// acquire is-dual property
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_FLAGS, 0);
	if(!pResource) {
#ifdef BT_CONFIG_OF
		if(dev) {
			const BT_be32 *is_dual = bt_of_get_property(dev, "is-dual", NULL);
			if(is_dual) {
				hQSPI->is_dual = bt_be32_to_cpu(*is_dual);
				goto is_dual_set;
			}
		}
#else
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_int_out;
#endif
	} else {
		hQSPI->is_dual = pResource->ulStart;
	}

#ifdef BT_CONFIG_OF
	is_dual_set:
#endif

	hQSPI->pSLCR = (ZYNQ_SLCR_REGS*) bt_ioremap((void*)ZYNQ_SLCR_BASE, sizeof(ZYNQ_SLCR_REGS));

	zynq_slcr_unlock(hQSPI->pSLCR);
	hQSPI->pSLCR->LQSPI_RST_CTRL = 3;
	hQSPI->pSLCR->LQSPI_RST_CTRL = 0;
	zynq_slcr_lock(hQSPI->pSLCR);

	/* QSPI controller initialization */
	qspi_init_hw(hQSPI,hQSPI->is_dual);

	hQSPI->done = BT_FALSE;

	// acquire bus-num
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_BUSID, 0);
	if(!pResource) {
			Error = BT_ERR_INVALID_RESOURCE;
			goto err_free_int_out;
	}
	hQSPI->spi_master.bus_num = pResource->ulStart;

	// acquire num-chip-select
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_NUM_CS, 0);
	if(!pResource) {
#ifdef BT_CONFIG_OF
		if(dev) {
		    const BT_be32 *num_cs = bt_of_get_property(dev, "num-chip-select", NULL);
			if(num_cs) {
				hQSPI->spi_master.num_chipselect = bt_be32_to_cpu(*num_cs);
				goto num_chipselect_set;
	        }
		}
#endif
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_int_out;
	}

	hQSPI->spi_master.num_chipselect = pResource->ulStart;

#ifdef BT_CONFIG_OF
	num_chipselect_set:
#endif

	// set quad mode
	hQSPI->spi_master.flags = SPI_MASTER_QUAD_MODE;

	BT_u32 clk_sel = ZYNQ_SLCR_CLK_CTRL_SRCSEL_VAL(hQSPI->pSLCR->LQSPI_CLK_CTRL);
	BT_u32 InputClk;
	switch(clk_sel) {
		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_ARMPLL:
			InputClk = BT_ZYNQ_GetArmPLLFrequency();
			break;
		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_IOPLL:
			InputClk = BT_ZYNQ_GetIOPLLFrequency();
			break;

		case ZYNQ_SLCR_CLK_CTRL_SRCSEL_DDRPLL:
			InputClk = BT_ZYNQ_GetDDRPLLFrequency();
			break;

		default:
			return NULL;
	}

	InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(hQSPI->pSLCR->LQSPI_CLK_CTRL);

	hQSPI->speed_hz = InputClk / 2;
	hQSPI->dev_busy = 0;

#if	(QSPI_USE_WORKQUEUE)
	BT_LIST_INIT_HEAD(&hQSPI->queue);

	hQSPI->queue_state = QSPI_QUEUE_STOPPED;

	BT_INIT_WORK(&hQSPI->work, qspi_work_queue);
	hQSPI->workqueue = bt_create_singlethread_workqueue(dev_name(&dev->dev));
	if (!hQSPI->workqueue) {
		ret = -1; // -ENOMEM
		BT_kPrint("problem initializing queue\n");
		goto err_free_mutex_transfer;
	}


	if(qspi_start_queue(hQSPI) != 0)
	{
		BT_kPrint("problem starting queue\n");

		goto remove_queue;

	}
#endif

	Error = BT_SpiRegisterMaster(hQSPI, &hQSPI->spi_master);
	if(Error != BT_ERR_NONE) {
		goto err_free_int_out;
	}

	if(pError) {
		*pError = Error;
	}

	BT_kPrint("ZYNQ-QSPI: at 0x%08X mapped to 0x%08X, irq=%d", mem_start, hQSPI->pRegs, hQSPI->irq);

	return hQSPI;

#if	(QSPI_USE_WORKQUEUE)
remove_queue:
	(void)qspi_destroy_queue(hQSPI);
#endif

err_free_int_out:
	BT_UnregisterInterrupt(ulIRQ, qspi_irq, hQSPI);

err_free_out:
	BT_DestroyHandle(hQSPI);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oQspiDriver = {
	.name 		= "xlnx,ps7-qspi-1.00.a",
	.pfnProbe 	= qspi_probe,
};
