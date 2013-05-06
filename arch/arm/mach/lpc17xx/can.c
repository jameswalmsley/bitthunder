/**
 * 	LPC17xx Hal for BitThunder
 *	CAN Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional CAN device driver
 *	for BitThunder.
 *
 *	This driver should be easily ported to CAN peripherals on other processors with little effort.
 *
 *	@author		Robert Steinbauer <rsteinbauer@riegl.com>
 *	@copyright	(c)2012 Robert Steinbauer
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>	 				// Include all those wonderful BitThunder APIs.
#include "can.h"						// Includes a full hardware definition for the integrated can devices.
#include "rcc.h"						// Used for getting access to rcc regs, to determine the real Clock Freq.
#include "ioconfig.h"						// Used for getting access to IOCON regs.
#include <string.h>
#include <collections/bt_fifo.h>

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("LPC17xx-CAN")
BT_DEF_MODULE_DESCRIPTION				("Simple Can device for the LPC17xx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a CAN driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC17xx_CAN_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_CAN_OPERATING_MODE	eMode;
	BT_HANDLE		   		hRxFifo;		///< RX fifo - ring buffer.
	BT_HANDLE		   		hTxFifo;		///< TX fifo - ring buffer.
};

static BT_HANDLE g_CAN_HANDLES[2] = {
	NULL,
	NULL,
};

static const BT_u32 g_CAN_PERIPHERAL[2] = {13, 14};

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the canOpen function.
static void disableCanPeripheralClock(BT_HANDLE hCan);

static BT_ERROR canDisable(BT_HANDLE hCan);
static BT_ERROR canEnable(BT_HANDLE hCan);
static void CanTransmit(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage);
static void CanReceive(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage);
static BT_u32 canFindFreeBuffer(BT_HANDLE hCan);


void isr_CAN(BT_HANDLE hCan) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;
	BT_ERROR Error;
	BT_CAN_MESSAGE oMessage;

	BT_u32 ulICR = pRegs->CANICR;

	if (ulICR & LPC17xx_CAN_ICR_RI) {
		CanReceive(g_CAN_HANDLES[0], &oMessage);
		BT_FifoWrite(hCan->hRxFifo, 1, &oMessage, &Error);
	}
	if (ulICR & LPC17xx_CAN_ICR_TI) {
		while (!BT_FifoIsEmpty(hCan->hTxFifo, &Error) && (canFindFreeBuffer(hCan) != LPC17xx_CAN_NO_FREE_BUFFER)) {
			BT_FifoRead(hCan->hTxFifo, 1, &oMessage, &Error);
			CanTransmit(hCan, &oMessage);
		}
	}
	if (BT_FifoIsEmpty(hCan->hTxFifo, &Error)) {
		pRegs->CANIER &= ~LPC17xx_CAN_IER_TIE;	// Disable the interrupt
	}
}

BT_ERROR BT_NVIC_IRQ_41(void) {
	if (g_CAN_HANDLES[0] != NULL) isr_CAN(g_CAN_HANDLES[0]);
	if (g_CAN_HANDLES[1] != NULL) isr_CAN(g_CAN_HANDLES[1]);

	return 0;
}

BT_ERROR BT_NVIC_IRQ_50(void) {
	//@@
	return 0;
}


/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR canCleanup(BT_HANDLE hCan) {
	canDisable(hCan);

	// Disable peripheral clock.
	disableCanPeripheralClock(hCan);

	// Free any buffers if used.
	if(hCan->eMode != BT_CAN_MODE_BUFFERED) {
		BT_CloseHandle(hCan->hRxFifo);
		BT_CloseHandle(hCan->hTxFifo);
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_IRQ, 0);

	BT_u32 i;
	BT_BOOL bClose = BT_TRUE;
	for (i = 0; i < 2; i++) {
		if ((hCan->pDevice->id != i) && (g_CAN_HANDLES[i] != NULL)) {
			bClose = BT_FALSE;
		}
	}

	if (bClose) BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_ENUM, 0);

	g_CAN_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

/**
 *	This actually allows the CAN devices to be clocked!
 **/
static void enableCanPeripheralClock(BT_HANDLE hCan) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_CAN1EN;
		break;
	}
	case 1: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_CAN2EN;
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	If the serial port is not in use, we can make it sleep!
 **/
static void disableCanPeripheralClock(BT_HANDLE hCan) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_CAN1EN;
		break;
	}
	case 1: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_CAN2EN;
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	Function to test the current peripheral clock gate status of the devices.
 **/
static BT_BOOL isCanPeripheralClockEnabled(BT_HANDLE hCan) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_CAN1EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_CAN2EN) {
			return BT_TRUE;
		}
		break;
	}
	default: {
		break;
	}
	}

	return BT_FALSE;
}

/**
 *	This implements the UART power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR canSetPowerState(BT_HANDLE hCan, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableCanPeripheralClock(hCan);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableCanPeripheralClock(hCan);
		break;
	}

	default: {
		//return BT_ERR_POWER_STATE_UNSUPPORTED;
		return (BT_ERROR) -1;
	}
	}

	return BT_ERR_NONE;
}

/**
 *	This implements the UART power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR canGetPowerState(BT_HANDLE hCan, BT_POWER_STATE *pePowerState) {
	if(isCanPeripheralClockEnabled(hCan)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}


static BT_ERROR canSetBaudrate(BT_HANDLE hCan, BT_u32 ulBaudrate) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	BT_u32 ulInputClk = BT_LPC17xx_GetPeripheralClock(g_CAN_PERIPHERAL[hCan->pDevice->id]);

	BT_u32 ulPrescale = 0;
	BT_u32 ulCanClock = ulInputClk/(ulPrescale+1);

	while ((ulCanClock >= ulBaudrate*8) && (ulCanClock % ulBaudrate == 0)) {
		ulPrescale++;
		ulCanClock = ulInputClk/(ulPrescale+1);
	}

	BT_u32 ulTSEG1 = (ulInputClk/(ulPrescale*ulBaudrate)-4);
	BT_u32 ulTSEG2 = 1;
	if (ulTSEG1 > 15) {
		BT_u32 ulDiff = ulTSEG1 - 15;
		ulTSEG2 += ulDiff;
		ulTSEG1 -= ulDiff;
	}

	pRegs->CANBTR = (ulTSEG1 << 16) | (ulTSEG2 << 20) | (ulPrescale-1);


	return BT_ERR_NONE;
};


/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR canSetConfig(BT_HANDLE hCan, BT_CAN_CONFIG *pConfig) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	canSetBaudrate(hCan, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hCan->eMode !=  BT_CAN_MODE_POLLED) {

			if(hCan->hTxFifo) {
				BT_CloseHandle(hCan->hTxFifo);
				hCan->hTxFifo = NULL;
			}
			if(hCan->hRxFifo) {
				BT_CloseHandle(hCan->hRxFifo);
				hCan->hRxFifo = NULL;
			}

			// Disable TX and RX interrupts
			pRegs->CANIER &= ~LPC17xx_CAN_IER_RIE;	// Disable the interrupt

			hCan->eMode = BT_CAN_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hCan->eMode != BT_CAN_MODE_BUFFERED) {
			if(!hCan->hRxFifo && !hCan->hTxFifo) {
				hCan->hRxFifo = BT_FifoCreate(pConfig->usRxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);
				hCan->hTxFifo = BT_FifoCreate(pConfig->usTxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);

				pRegs->CANIER |= LPC17xx_CAN_IER_RIE;	// Enable the interrupt
				hCan->eMode = BT_CAN_MODE_BUFFERED;
			}
		}
		break;
	}
	}

	LPC17xx_CAN_COMMON_REGS *pCMN = CAN_COMMON;

	pCMN->LPC17xx_CAN_AFMR |= LPC17xx_CAN_AFMR_ACCBP;

	return Error;
}

/**
 *	Get a full configuration of the UART.
 **/
static BT_ERROR canGetConfig(BT_HANDLE hCan, BT_CAN_CONFIG *pConfig) {
	return BT_ERR_NONE;
}

/**
 *	Make the CAN active (Set the Enable bit).
 **/
static BT_ERROR canEnable(BT_HANDLE hCan) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	pRegs->CANMOD &= ~LPC17xx_CAN_MOD_RM;

	return BT_ERR_NONE;
}

/**
 *	Make the CAN inactive (Clear the Enable bit).
 **/
static BT_ERROR canDisable(BT_HANDLE hCan) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	pRegs->CANMOD |= LPC17xx_CAN_MOD_RM;

	return BT_ERR_NONE;
}


static BT_ERROR canRead(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hCan->eMode) {
	case BT_CAN_MODE_POLLED:
	{
		while(!(pRegs->CANGSR & LPC17xx_CAN_GSR_RBS)) {
			BT_ThreadYield();
		}
		CanReceive(hCan, pCanMessage);
		break;
	}

	case BT_CAN_MODE_BUFFERED:
	{
		// Get bytes from RX buffer very quickly.
		BT_FifoRead(hCan->hRxFifo, 1, pCanMessage, &Error);
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;
	}
	return Error;
}

/**
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR canWrite(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {
	volatile LPC17xx_CAN_REGS *pRegs = hCan->pRegs;
	BT_CAN_MESSAGE oMessage;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hCan->eMode) {
	case BT_CAN_MODE_POLLED:
	{
		BT_u32 ulIndex;
		while ((ulIndex = canFindFreeBuffer(hCan)) == LPC17xx_CAN_NO_FREE_BUFFER) {
			BT_ThreadYield();
		}
		CanTransmit(hCan, pCanMessage);
		break;
	}

	case BT_CAN_MODE_BUFFERED:
	{
		BT_FifoWrite(hCan->hTxFifo, 1, pCanMessage, &Error);

		while (!BT_FifoIsEmpty(hCan->hTxFifo, &Error) && (canFindFreeBuffer(hCan) != LPC17xx_CAN_NO_FREE_BUFFER)) {
			BT_FifoRead(hCan->hTxFifo, 1, &oMessage, &Error);
			CanTransmit(hCan, &oMessage);
		}

		pRegs->CANIER |= LPC17xx_CAN_IER_TIE;	// Enable the interrupt

		break;
	}

	default:
		break;
	}
	return Error;
}

static void CanTransmit(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {
	LPC17xx_CAN_REGS *pRegs = hCan->pRegs;
	BT_u32			  ulIndex = canFindFreeBuffer(hCan);

	LPC17xx_CAN_BUFFER *pBuf = &pRegs->CANTMTBuf[ulIndex];

	pBuf->CANID = pCanMessage->ulID;
	pBuf->CANFS = pCanMessage->ucLength << 16;
	if (pCanMessage->ulID & BT_CAN_EFF_FLAG) {
		pBuf->CANFS |= LPC17xx_CAN_FS_FF;
	}
	union {
		BT_u32	ulCANData[2];
		BT_u8	ucCANData[8];
	} LPC17xx_CAN_DATA;

	BT_u32 i;
	for (i=0; i<pCanMessage->ucLength; i++) {
		LPC17xx_CAN_DATA.ucCANData[i] = pCanMessage->ucdata[i];
	}
	pBuf->CANData[0] = LPC17xx_CAN_DATA.ulCANData[0];
	pBuf->CANData[1] = LPC17xx_CAN_DATA.ulCANData[1];

	pRegs->CANCMR |= LPC17xx_CAN_CMR_SELECT_TXBUF << ulIndex;
	pRegs->CANCMR |= LPC17xx_CAN_CMR_TR;
}

static void CanReceive(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {
	LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	LPC17xx_CAN_BUFFER *pBuf = &pRegs->CANRCVBuf[0];

	pCanMessage->ulID = pBuf->CANID;
	pCanMessage->ucLength = LPC17xx_CAN_FS_DLC(pBuf->CANFS);

	if (pBuf->CANFS & LPC17xx_CAN_FS_FF)
		pCanMessage->ulID |= BT_CAN_EFF_FLAG;

	memcpy(pCanMessage->ucdata, pBuf->CANData, pCanMessage->ucLength);

	pRegs->CANCMR |= LPC17xx_CAN_CMR_RRB;
}

static BT_u32 canFindFreeBuffer(BT_HANDLE hCan) {
	LPC17xx_CAN_REGS *pRegs = hCan->pRegs;

	if (pRegs->CANSR & LPC17xx_CAN_SR_TBS1) return 0;
	if (pRegs->CANSR & LPC17xx_CAN_SR_TBS2) return 1;
	if (pRegs->CANSR & LPC17xx_CAN_SR_TBS3) return 2;
	return LPC17xx_CAN_NO_FREE_BUFFER;
}

static const BT_DEV_IF_CAN oCanConfigInterface = {
	.pfnSetBaudrate	= canSetBaudrate,
	.pfnSetConfig	= canSetConfig,											///< CAN set config imple.
	.pfnGetConfig	= canGetConfig,
	.pfnEnable	 	= canEnable,												///< Enable/disable the device.
	.pfnDisable	 	= canDisable,
	.pfnSendMessage	= canWrite,
	.pfnReadMessage	= canRead,
};

static const BT_IF_POWER oPowerInterface = {
	.pfnSetPowerState	= canSetPowerState,											///< Pointers to the power state API implementations.
	.pfnGetPowerState	= canGetPowerState,											///< This gets the current power state.
};


static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oCanConfigInterface,
};

const BT_IF_DEVICE BT_LPC17xx_CAN_oDeviceInterface = {
	&oPowerInterface,											///< Device does not support powerstate functionality.
	BT_DEV_IF_T_CAN,											///< Allow configuration through the CAN api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oCanConfigInterface,
	},
	NULL,											///< Provide a Character device interface implementation.
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC17xx_CAN_oDeviceInterface,
	},
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	canCleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE can_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hCan = NULL;


	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_CAN_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hCan = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hCan) {
		goto err_out;
	}

	g_CAN_HANDLES[pResource->ulStart] = hCan;

	hCan->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hCan->pRegs = (LPC17xx_CAN_REGS *) pResource->ulStart;

	canSetPowerState(hCan, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults!

	canDisable(hCan);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, uart_irq_handler, hCan);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hCan;

err_free_out:
	BT_kFree(hCan);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF can_driver = {
	.name 		= "LPC17xx,can",
	.pfnProbe	= can_probe,
};


#ifdef BT_CONFIG_MACH_LPC17xx_CAN_0
static const BT_RESOURCE oLPC17xx_can0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_CAN0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_CAN0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 41,
		.ulEnd				= 41,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_can0_device = {
	.id						= 0,
	.name 					= "LPC17xx,can",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_can0_resources),
	.pResources 			= oLPC17xx_can0_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_can0_inode = {
	.szpName = "can0",
	.pDevice = &oLPC17xx_can0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_CAN_1
static const BT_RESOURCE oLPC17xx_can1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_CAN1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_CAN1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 41,
		.ulEnd				= 41,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_can1_device = {
	.id						= 1,
	.name 					= "LPC17xx,can",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_can1_resources),
	.pResources 			= oLPC17xx_can1_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_can1_inode = {
	.szpName = "can1",
	.pDevice = &oLPC17xx_can1_device,
};
#endif
