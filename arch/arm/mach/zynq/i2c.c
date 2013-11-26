#include <bitthunder.h>
#include <collections/bt_fifo.h>
#include "i2c.h"
#include "slcr.h"

BT_DEF_MODULE_NAME				("Zynq-I2C")
BT_DEF_MODULE_DESCRIPTION		("I2C device driver for Zynq")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

#define I2C_ADDR_MASK			0x000003FF
#define I2C_ENABLED_INTERRUPTS	0x000002EF

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	BT_I2C_BUS					i2c_master;
	volatile ZYNQ_I2C_REGS 	   *pRegs;
	void					   *pMutex;
	BT_u32						err_status;
	BT_I2C_MESSAGE 			   *p_msg;
	BT_u8 					   *p_recv_buf;
	BT_u8 					   *p_send_buf;
	BT_u32						recv_count;
	BT_u32						send_count;
	BT_BOOL						bHold;
};

static BT_ERROR i2c_irq_handler(BT_u32 ulIRQ, void *pParam) {

	BT_HANDLE hI2C = (BT_HANDLE) pParam;
	BT_u32 isr_status = hI2C->pRegs->INT_STATUS;

	BT_u32 avail_bytes;
	BT_u32 bytes_to_recv, bytes_to_send;

	BT_BOOL bRelease = BT_FALSE;

	if(isr_status & INT_MASK_NACK) {
		// NACK
		bRelease = BT_TRUE;
	}

	if(isr_status & INT_MASK_ARB_LOST) {
		// BUS Arbitration was lost.
		bRelease = BT_TRUE;
	}

	if(isr_status & INT_MASK_DATA) {
		if(hI2C->p_msg->len > I2C_FIFO_LEN) {
			bytes_to_recv = (I2C_FIFO_LEN + 1) - hI2C->pRegs->TRANSFER_SIZE;
			hI2C->p_msg->len -= bytes_to_recv;

			if(hI2C->p_msg->len > I2C_FIFO_LEN) {
				hI2C->pRegs->TRANSFER_SIZE = I2C_FIFO_LEN + 1;
			} else {
				hI2C->pRegs->TRANSFER_SIZE = hI2C->p_msg->len;
				if(!hI2C->bHold) {
					hI2C->pRegs->CONTROL &= CONTROL_HOLD;
				}
			}

			while(bytes_to_recv) {
				*hI2C->p_recv_buf++ = hI2C->pRegs->DATA;
				bytes_to_recv--;
			}

		}
	}

	if(isr_status & INT_MASK_COMP) {
		if(!hI2C->p_recv_buf) {
			if(hI2C->send_count > 0) {
				avail_bytes = I2C_FIFO_LEN - hI2C->pRegs->TRANSFER_SIZE;
				if(hI2C->send_count > avail_bytes) {
					bytes_to_send = avail_bytes;
				} else {
					bytes_to_send = hI2C->send_count;
				}

				while(bytes_to_send--) {
					hI2C->pRegs->DATA = *hI2C->p_send_buf++;
					hI2C->send_count--;
				}
			} else {
				bRelease = BT_TRUE;
			}

			if(!hI2C->send_count) {
				if(!hI2C->bHold) {
					hI2C->pRegs->CONTROL &= ~CONTROL_HOLD;
				}
			}
		} else {
			if(!hI2C->bHold) {
				hI2C->pRegs->CONTROL &= ~CONTROL_HOLD;
			}

			while(hI2C->pRegs->STATUS & STATUS_RXDV) {
				*hI2C->p_recv_buf++ = hI2C->pRegs->DATA;
				hI2C->recv_count--;
			}
			bRelease = BT_TRUE;
		}
	}

	if(bRelease) {
		BT_kMutexReleaseFromISR(hI2C->pMutex, NULL);
	}

	hI2C->err_status = isr_status & 0x000002EC;
	hI2C->pRegs->INT_STATUS = isr_status;			// Clear pending interrupts.

	return BT_ERR_NONE;
}

static BT_ERROR i2c_cleanup(BT_HANDLE hI2C) {

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->i2c_master.pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		return BT_ERR_GENERIC;
	}

	BT_ERROR Error = BT_UnregisterInterrupt(pResource->ulStart, i2c_irq_handler, hI2C);

	BT_kMutexDestroy(hI2C->pMutex);

	return Error;
}

static BT_ERROR i2c_set_clock_rate(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockRate) {

	BT_u32 clk = BT_ZYNQ_GetCpu1xFrequency();
	BT_u32 target;

	switch(eClockRate) {
	case BT_I2C_CLOCKRATE_100kHz:
		target = 100000;
		break;

	case BT_I2C_CLOCKRATE_400kHz:
		target = 400000;
		break;

	case BT_I2C_CLOCKRATE_1000kHz:	///< Unsupported Freq!
	case BT_I2C_CLOCKRATE_3400kHz:	///< Unsupported Freq!
	default:
		return BT_ERR_GENERIC;
		break;
	}

	BT_DIVIDER_PARAMS oDiv;
	oDiv.diva_max = 4;
	oDiv.diva_min = 0;
	oDiv.divb_max = 64;
	oDiv.divb_min = 0;

	BT_CalculateClockDivider(clk/22, target, &oDiv);

	CONTROL_DIV_A_SET(hI2C->pRegs->CONTROL, (oDiv.diva_val-1));
	CONTROL_DIV_B_SET(hI2C->pRegs->CONTROL, (oDiv.divb_val-1));

	BT_kPrint("Found a divider: a=%d, b=%d, clkout= %d Hz", oDiv.diva_val, oDiv.divb_val, oDiv.clk_out);

	return BT_ERR_NONE;
}

static void mrecv(BT_HANDLE hI2C) {

	hI2C->p_send_buf = NULL;
	hI2C->p_recv_buf = hI2C->p_msg->buf;
	hI2C->recv_count = hI2C->p_msg->len;

	hI2C->pRegs->CONTROL |= (CONTROL_RW | CONTROL_CLR_FIFO);

	if(hI2C->recv_count > I2C_FIFO_LEN) {
		hI2C->pRegs->CONTROL |= CONTROL_HOLD;
	}

	// Clear Interrupts
	hI2C->pRegs->INT_STATUS = hI2C->pRegs->INT_STATUS;

	hI2C->pRegs->ADDRESS = hI2C->p_msg->addr & I2C_ADDR_MASK;

	if(hI2C->recv_count > I2C_FIFO_LEN) {
		hI2C->pRegs->TRANSFER_SIZE = I2C_FIFO_LEN + 1;
	} else {

		hI2C->pRegs->TRANSFER_SIZE = hI2C->recv_count;

		if(!hI2C->bHold && !(hI2C->p_msg->flags & BT_I2C_M_RECV_LEN)) {
			hI2C->pRegs->CONTROL &= ~CONTROL_HOLD;
		}
	}

	hI2C->pRegs->INT_ENABLE = I2C_ENABLED_INTERRUPTS;
}

static void msend(BT_HANDLE hI2C) {

	BT_u32 avail_bytes;
	BT_u32 bytes_to_send;

	hI2C->p_recv_buf = NULL;
	hI2C->p_send_buf = hI2C->p_msg->buf;
	hI2C->send_count = hI2C->p_msg->len;

	hI2C->pRegs->CONTROL &= ~CONTROL_RW;
	hI2C->pRegs->CONTROL |= CONTROL_CLR_FIFO;

	if(hI2C->send_count > I2C_FIFO_LEN) {
		hI2C->pRegs->CONTROL |= CONTROL_HOLD;
	}

	// Clear interrupts,
	hI2C->pRegs->INT_STATUS = hI2C->pRegs->INT_STATUS;

	avail_bytes = I2C_FIFO_LEN - hI2C->pRegs->TRANSFER_SIZE;

	if(hI2C->send_count > avail_bytes) {
		bytes_to_send = avail_bytes;
	} else {
		bytes_to_send = hI2C->send_count;
	}

	while(bytes_to_send--) {
		hI2C->pRegs->DATA = *hI2C->p_send_buf++;
		hI2C->send_count -= 1;
	}

	hI2C->pRegs->ADDRESS = hI2C->p_msg->addr & I2C_ADDR_MASK;

	if(!hI2C->bHold && hI2C->send_count == 0) {
		hI2C->pRegs->CONTROL &= ~CONTROL_HOLD;
	}

	hI2C->pRegs->INT_ENABLE = I2C_ENABLED_INTERRUPTS;
}

static BT_u32 i2c_master_transfer(BT_HANDLE hI2C, BT_I2C_MESSAGE *msgs, BT_u32 num, BT_ERROR *pError) {

	while(hI2C->pRegs->STATUS & STATUS_BA) {
		BT_ThreadYield();
	}

	if(num > 1) {
		hI2C->bHold = 1;
		hI2C->pRegs->CONTROL |= CONTROL_HOLD;
	} else {
		hI2C->bHold = BT_FALSE;
	}

	BT_u32 count;
	for(count = 0; count < num; count++, msgs++) {
		if(count == (num - 1)) {
			hI2C->bHold = BT_FALSE;
		}

retry:
		hI2C->err_status = 0;
		hI2C->p_msg = msgs;

		if(msgs->flags & BT_I2C_M_TEN) {
			hI2C->pRegs->CONTROL &= ~CONTROL_NEA;
		} else {
			hI2C->pRegs->CONTROL |= CONTROL_NEA;
		}

		if(msgs->flags & BT_I2C_M_RD) {
			mrecv(hI2C);
		} else {
			msend(hI2C);
		}

		// Wait on transfer complete signal.
		BT_kMutexPend(hI2C->pMutex, 0);
		hI2C->pRegs->INT_DISABLE = 0x000002FF;

		if(hI2C->err_status & INT_MASK_ARB_LOST) {
			BT_kPrint("ZYNQ I2C: Lost ownership on bus, trying again...");
			BT_ThreadSleep(2);
			goto retry;
		}

		if(hI2C->err_status & INT_MASK_NACK) {
			//BT_kPrint("ZYNQ I2C: NACK occured!");
			break;
		}
	}

	hI2C->p_msg = NULL;
	hI2C->err_status = 0;

	return count;
}

static const BT_DEV_IF_I2C oI2CInterface = {
	.ulFunctionality	= BT_I2C_FUNC_I2C | BT_I2C_FUNC_10BIT_ADDR,
	.pfnMasterTransfer	= i2c_master_transfer,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pPowerIF = NULL,
	.eConfigType = BT_DEV_IF_T_I2C,
	.unConfigIfs = {
		.pI2CIF = &oI2CInterface,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.eType = BT_HANDLE_T_DEVICE,
	.pfnCleanup = i2c_cleanup,
};

static BT_HANDLE i2c_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hI2C = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_out;
	}

	hI2C = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hI2C) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hI2C->pRegs = (ZYNQ_I2C_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_4K);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	BT_u32 ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(ulIRQ, i2c_irq_handler, hI2C);
	if(Error) {
		Error = BT_ERR_GENERIC;	// Device already in use!
		goto err_free_out;
	}

	BT_EnableInterrupt(ulIRQ);

	hI2C->i2c_master.pDevice = pDevice;
	i2c_set_clock_rate(hI2C, BT_I2C_CLOCKRATE_400kHz);

	hI2C->pMutex = BT_kMutexCreate();
	if(!hI2C->pMutex) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_int_out;
	}

	BT_kMutexPend(hI2C->pMutex, 0);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_BUSID, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_int_out;
	}

	BT_u32 ulBusID = pResource->ulStart;

	hI2C->i2c_master.ulBusID	= ulBusID;
	hI2C->i2c_master.hBus 		= hI2C;		// Save pointer to bus's private data.
	hI2C->i2c_master.pDevice 	= pDevice;

	hI2C->pRegs->CONTROL |= 0x0000000E;

	BT_I2C_RegisterBus(&hI2C->i2c_master);

	if(pError) {
		*pError = Error;
	}

	return hI2C;

err_free_int_out:
	BT_UnregisterInterrupt(ulIRQ, i2c_irq_handler, hI2C);

err_free_out:
	BT_DestroyHandle(hI2C);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 		= "xlnx,ps7-i2c-1.00.a",
	.pfnProbe 	= i2c_probe,
};
