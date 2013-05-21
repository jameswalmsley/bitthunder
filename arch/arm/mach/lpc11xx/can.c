/**
 * 	LPC11xx Hal for BitThunder
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
BT_DEF_MODULE_NAME						("LPC11xx-CAN")
BT_DEF_MODULE_DESCRIPTION				("Simple Can device for the LPC11xx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a CAN driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC11xx_CAN_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_HANDLE		   		hRxFifo;		///< RX fifo - ring buffer.
	BT_HANDLE		   		hTxFifo;		///< TX fifo - ring buffer.
};

static BT_HANDLE g_CAN_HANDLES[1] = {
	NULL,
};

static BT_u8 g_Msg_Queue[1] = {
	0,
};

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the canOpen function.
static void disableCanPeripheralClock(BT_HANDLE hCan);

static BT_ERROR canDisable(BT_HANDLE hCan);
static BT_ERROR canEnable(BT_HANDLE hCan);
static void CanTransmit(BT_HANDLE hCan);


/*
 * ROM handlers
 */
typedef	struct _LPC11xx_CAN_CALLBACKS {
  void (*CAN_rx)(BT_u8 ucmsg_obj_num);
  void (*CAN_tx)(BT_u8 ucmsg_obj_num);
  void (*CAN_error)(BT_u32 ulerror_info);
  BT_u32 (*CANOPEN_sdo_read)(BT_u16 usindex, BT_u8 ucsubindex);
  BT_u32 (*CANOPEN_sdo_write)(BT_u16 usindex, BT_u8 ucsubindex, BT_u8 *pdat_ptr);
  BT_u32 (*CANOPEN_sdo_seg_read)(BT_u16 usindex, BT_u8 ucsubindex, BT_u8 ucopenclose, BT_u8 *plength, BT_u8 *pdata, BT_u8 *plast);
  BT_u32 (*CANOPEN_sdo_seg_write)(BT_u16 usindex, BT_u8 ucsubindex, BT_u8 ucopenclose, BT_u8 uclength, BT_u8 *pdata, BT_u8 *pfast_resp);
  BT_u8 (*CANOPEN_sdo_req)(BT_u8 uclength_req, BT_u8 *preq_ptr, BT_u8 *plength_resp, BT_u8 *presp_ptr);
} LPC11xx_CAN_CALLBACKS;

typedef	struct _LPC11xx_CAND {
  void (*init_can)(BT_u32 *pcan_cfg, BT_u8 ucisr_ena);
  void (*isr)(void);
  void (*config_rxmsgobj)(LPC11xx_CAN_MSG_OBJ *pmsg_obj);
  BT_u8 (*can_receive)(LPC11xx_CAN_MSG_OBJ *pmsg_obj);
  void (*can_transmit)(LPC11xx_CAN_MSG_OBJ *pmsg_obj);
  void (*config_canopen)(LPC11xx_CAN_CANOPENCFG *pcanopen_cfg);
  void (*canopen_handler)(void);
  void (*config_calb)(LPC11xx_CAN_CALLBACKS *pcallback_cfg);
} LPC11xx_CAND;


#define	BT_CONFIG_DRIVER_ROMCAN		1

typedef	struct _ROM {
   const unsigned p_usbd;
   const unsigned p_clib;

#if BT_CONFIG_DRIVER_ROMCAN==1
   const LPC11xx_CAND *pCAND;
#else
   const unsigned pCAND;
#endif /* (PERIPHERAL == CAN) */

#ifdef BT_CONFIG_DRIVER_ROMPWR
   const PWRD * pPWRD;
#else
   const unsigned p_pwrd;
#endif /* PWRROMD_PRESENT */
   const unsigned p_dev1;
   const unsigned p_dev2;
   const unsigned p_dev3;
   const unsigned p_dev4;
}  ROM;

ROM **rom = (ROM **)0x1fff1ff8;

void CAN_rx(BT_u8 ucmsg_obj_num);
void CAN_tx(BT_u8 ucmsg_obj_num);
void CAN_error(BT_u32 ulerror_info);

LPC11xx_CAN_CALLBACKS callbacks = {
	CAN_rx,
	CAN_tx,
	CAN_error,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


void CAN_rx(BT_u8 ucmsg_obj_num) {
	LPC11xx_CAN_MSG_OBJ omsg_obj;
	BT_HANDLE hCan = g_CAN_HANDLES[0];

	BT_ERROR Error = BT_ERR_NONE;

	omsg_obj.ucmsgobj = ucmsg_obj_num;

	(*rom)->pCAND->can_receive(&omsg_obj);

	BT_CAN_MESSAGE oMessage;

	oMessage.ulID	  = omsg_obj.ulmodeid & ~LPC11xx_CAN_MSGOBJ_EXT;
	if (omsg_obj.ulmodeid & LPC11xx_CAN_MSGOBJ_EXT) {
		oMessage.ulID |= BT_CAN_EFF_FLAG;
	}
	oMessage.ucLength = omsg_obj.uclength;

	memcpy(oMessage.ucdata, omsg_obj.ucdata, oMessage.ucLength);

	BT_FifoWrite(hCan->hRxFifo, 1, &oMessage, &Error);
}

void CAN_tx(BT_u8 ucmsg_obj_num) {

	g_Msg_Queue[0] = 0;
	CanTransmit(g_CAN_HANDLES[0]);

	return;
}

void CAN_error(BT_u32 ulerror_info) {
	return;
}

/*
 *
 */

void BT_NVIC_IRQ_29(void) {
	(*rom)->pCAND->isr();
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
	BT_CloseHandle(hCan->hRxFifo);
	BT_CloseHandle(hCan->hTxFifo);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_ENUM, 0);

	g_CAN_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	g_Msg_Queue[0] = 0;

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
		LPC11xx_RCC->SYSAHBCLKCTRL |= LPC11xx_RCC_SYSAHBCLKCTRL_CANEN;
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
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~LPC11xx_RCC_SYSAHBCLKCTRL_CANEN;
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
		if(LPC11xx_RCC->SYSAHBCLKCTRL & LPC11xx_RCC_SYSAHBCLKCTRL_CANEN) {
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
	BT_u32 ulClkInitTable[2] = {0x00000000, 0x00000000};

	BT_u32 ulInputClk = BT_GetCpuClockFrequency();

	ulClkInitTable[0] = (ulInputClk / (8 * ulBaudrate)) - 1;
	ulClkInitTable[1] = 0x2300;

	(*rom)->pCAND->init_can(&ulClkInitTable[0], 1);

	return BT_ERR_NONE;
};


/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR canSetConfig(BT_HANDLE hCan, BT_CAN_CONFIG *pConfig) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_IRQ, 0);
	BT_DisableInterrupt(pResource->ulStart);

	BT_ERROR Error = BT_ERR_NONE;

	canSetBaudrate(hCan, pConfig->ulBaudrate);

	if(!hCan->hRxFifo && !hCan->hTxFifo) {
		hCan->hRxFifo = BT_FifoCreate(pConfig->usRxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);
		hCan->hTxFifo = BT_FifoCreate(pConfig->usTxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);
	}

	(*rom)->pCAND->config_calb(&callbacks);

	/* Configure message object 0 to receive all 11-bit messages 0x000-0x7FF */
	LPC11xx_CAN_MSG_OBJ oMsg_Obj;

	oMsg_Obj.ucmsgobj = 0;
	oMsg_Obj.ulmodeid = 0x00000000;
	oMsg_Obj.ulmask = 0x00000000;
	(*rom)->pCAND->config_rxmsgobj(&oMsg_Obj);

	/* Configure message object 1 to receive all 29-bit messages 0x000-0x7FF */
	oMsg_Obj.ucmsgobj = 1;
	oMsg_Obj.ulmodeid = 0x20000000;
	oMsg_Obj.ulmask = 0x00000000;
	(*rom)->pCAND->config_rxmsgobj(&oMsg_Obj);

	BT_EnableInterrupt(pResource->ulStart);

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
	volatile LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	pRCC->PRESETCTRL |= LPC11xx_RCC_PRESETCTRL_CAN_DEASSERT;

	return BT_ERR_NONE;
}

/**
 *	Make the CAN inactive (Clear the Enable bit).
 **/
static BT_ERROR canDisable(BT_HANDLE hCan) {
	volatile LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	pRCC->PRESETCTRL &= ~LPC11xx_RCC_PRESETCTRL_CAN_DEASSERT;

	return BT_ERR_NONE;
}


static BT_ERROR canRead(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {
	// Get message from RX buffer very quickly.
	BT_ERROR Error = BT_ERR_NONE;

	BT_FifoRead(hCan->hRxFifo, 1, pCanMessage, &Error);

	return Error;
}

/**
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR canWrite(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_FifoWrite(hCan->hTxFifo, 1, pCanMessage, &Error);

	if (!g_Msg_Queue[0]) {
		CanTransmit(hCan);
	}

	return Error;
}

static void CanTransmit(BT_HANDLE hCan) {
	LPC11xx_CAN_MSG_OBJ oMsgObj;
	BT_CAN_MESSAGE oMessage;
	BT_ERROR Error = BT_ERR_NONE;

	if (!BT_FifoIsEmpty(hCan->hTxFifo, &Error)) {

		BT_FifoRead(hCan->hTxFifo, 1, &oMessage, &Error);
		oMsgObj.ucmsgobj = 2;
		oMsgObj.uclength = oMessage.ucLength;
		oMsgObj.ulmask   = 0x0000;
		oMsgObj.ulmodeid = oMessage.ulID & ~BT_CAN_EFF_FLAG;

		if (oMessage.ulID & BT_CAN_EFF_FLAG) {
			oMsgObj.ulmodeid |= LPC11xx_CAN_MSGOBJ_EXT;
		}

		memcpy(oMsgObj.ucdata, oMessage.ucdata, oMessage.ucLength);
		g_Msg_Queue[hCan->pDevice->id] = 1;


		(*rom)->pCAND->can_transmit(&oMsgObj);
	}
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

const BT_IF_DEVICE BT_LPC11xx_CAN_oDeviceInterface = {
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
		(BT_HANDLE_INTERFACE) &BT_LPC11xx_CAN_oDeviceInterface,
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

	hCan->pRegs = (LPC11xx_CAN_REGS *) pResource->ulStart;

	canSetPowerState(hCan, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults!

	canEnable(hCan);

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
	BT_DestroyHandle(hCan);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF can_driver = {
	.name 		= "LPC11xx,can",
	.pfnProbe	= can_probe,
};


#ifdef BT_CONFIG_MACH_LPC11xx_CAN_0
static const BT_RESOURCE oLPC11xx_can0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC11xx_CAN0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC11xx_CAN0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 13,
		.ulEnd				= 13,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC11xx_can0_device = {
	.id						= 0,
	.name 					= "LPC11xx,can",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_can0_resources),
	.pResources 			= oLPC11xx_can0_resources,
};

const BT_DEVFS_INODE_DEF oLPC11xx_can0_inode = {
	.szpName = "can0",
	.pDevice = &oLPC11xx_can0_device,
};
#endif
