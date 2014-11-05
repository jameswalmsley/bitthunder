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
	BT_u32					id;
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


#ifdef BT_CONFIG_MACH_USE_CAN_ROMDRIVER
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

	BT_FifoWriteFromISR(hCan->hRxFifo, 1, &oMessage);
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
#else

void CAN_MessageProcess(BT_HANDLE hCan, BT_u8 ucMsgNo ) {
	volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;

	BT_CAN_MESSAGE oMessage;
	BT_ERROR Error;

	while ( pRegs->CANIF2_CMDREQ & LPC11xx_CAN_IFCREQ_BUSY );
	pRegs->CANIF2_CMDMSK = LPC11xx_CAN_IFCMDMSK_RD |
						   LPC11xx_CAN_IFCMDMSK_MASK |
						   LPC11xx_CAN_IFCMDMSK_ARB |
						   LPC11xx_CAN_IFCMDMSK_CTRL |
						   LPC11xx_CAN_IFCMDMSK_INTPND |
						   LPC11xx_CAN_IFCMDMSK_TREQ |
						   LPC11xx_CAN_IFCMDMSK_DATAA |
						   LPC11xx_CAN_IFCMDMSK_DATAB;
	pRegs->CANIF2_CMDREQ = ucMsgNo+1;    /* Start message transfer */
	while ( pRegs->CANIF2_CMDREQ & LPC11xx_CAN_IFCREQ_BUSY );	/* Check new data bit */

	if( pRegs->CANIF2_ARB2 & LPC11xx_CAN_ID_MTD ) {	/* bit 28-0 is 29 bit extended frame */
		/* mask off MsgVal and Dir */
		oMessage.ulID = (pRegs->CANIF2_ARB1 | ((pRegs->CANIF2_ARB2 & 0x1FFF) << 16));
		oMessage.ulID |= BT_CAN_EFF_FLAG;
	}
	else {
		/* bit 28-18 is 11-bit standard frame */
		oMessage.ulID = (pRegs->CANIF2_ARB2 &0x1FFF) >> 2;
	}

	oMessage.ucLength = pRegs->CANIF2_MCTRL & 0x000F;	// Get Msg Obj Data length

	BT_u16 usBuffer[4];
	usBuffer[0] = pRegs->CANIF2_DA1 & 0x0000FFFF;
	usBuffer[1] = pRegs->CANIF2_DA2 & 0x0000FFFF;
	usBuffer[2] = pRegs->CANIF2_DB1 & 0x0000FFFF;
	usBuffer[3] = pRegs->CANIF2_DB2 & 0x0000FFFF;

	memcpy(oMessage.ucdata, usBuffer, 8);

	BT_FifoWriteFromISR(hCan->hRxFifo, 1, &oMessage);

	return;
}


void BT_NVIC_IRQ_29(void) {
	BT_HANDLE hCan = g_CAN_HANDLES[0];

	volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;

	volatile BT_u32 canstat = 0;
	volatile BT_u32 can_int, msg_no;


	while ((can_int = pRegs->CANINT) != 0 ) {
		canstat = pRegs->CANSTAT;
		if ( can_int & LPC11xx_CAN_INT_STATUS_INTERRUPT ) {
			if ( canstat & (LPC11xx_CAN_STAT_EWARN | LPC11xx_CAN_STAT_BOFF) ) {
				return;
			}
		}
		else {
			if ( (canstat & LPC11xx_CAN_STAT_LEC) == 0 ) { 	/* NO ERROR */
				/* deal with RX only for now. */
				msg_no = can_int & 0x7FFF;
				if ( (msg_no >= 0x01) && (msg_no <= 0x20) ) {
					pRegs->CANSTAT &= ~LPC11xx_CAN_STAT_RXOK;
					CAN_MessageProcess(hCan,  msg_no-1);
				}
			}
		}
	}
	return;
}
#endif


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
	BT_u32 ulInputClk = BT_GetCpuClockFrequency();

#ifdef BT_CONFIG_MACH_USE_CAN_ROMDRIVER
	BT_u32 ulClkInitTable[2] = {0x00000000, 0x00000000};

	ulClkInitTable[0] = (ulInputClk / (8 * ulBaudrate)) - 1;
	ulClkInitTable[1] = 0x2300;

	(*rom)->pCAND->init_can(&ulClkInitTable[0], 1);
#else

	volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;

	/* Be very careful with this setting because it's related to
	the input bitclock setting value in CANBitClk. */
	/* popular CAN clock setting assuming AHB clock is 48Mhz:
	CLKDIV = 1, CAN clock is 48Mhz/2 = 24Mhz
	CLKDIV = 2, CAN clock is 48Mhz/3 = 16Mhz
	CLKDIV = 3, CAN clock is 48Mhz/4 = 12Mhz
	CLKDIV = 5, CAN clock is 48Mhz/6 = 8Mhz */

	/* AHB clock is 48Mhz, the CAN clock is 1/6 AHB clock = 8Mhz */
	pRegs->CANCLKDIV = ((ulInputClk / (8 * ulBaudrate)) - 1);			/* Divided by 6 */

	/* Start configuring bit timing */
	pRegs->CANCNTL |= LPC11xx_CAN_CTRL_CCE;
	pRegs->CANBT = 0x2300;
	pRegs->CANBRPE = 0x0000;
	/* Stop configuring bit timing */
	pRegs->CANCNTL &= ~LPC11xx_CAN_CTRL_CCE;
#endif

	return BT_ERR_NONE;
};


void canConfigureMessages(BT_HANDLE hCan) {
	volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;

	BT_u32 i;
	BT_u32 ext_frame = 0;

	/* It's organized in such a way that:
	obj(x+0)	standard	transmit
	obj(x+1)	standard	receive
	obj(x+2)	extended	transmit	where x is 0 to 3
	obj(x+3)	extended	receive*/

	for ( i = 0; i < 4; i++ ) {
		pRegs->CANIF1_CMDMSK = LPC11xx_CAN_IFCMDMSK_WR |
							   LPC11xx_CAN_IFCMDMSK_MASK |
							   LPC11xx_CAN_IFCMDMSK_ARB |
							   LPC11xx_CAN_IFCMDMSK_CTRL |
							   LPC11xx_CAN_IFCMDMSK_DATAA |
							   LPC11xx_CAN_IFCMDMSK_DATAB;

		pRegs->CANIF1_MSK1 = 0x0000;
		pRegs->CANIF1_MSK2 = 0x0000;

		pRegs->CANIF1_ARB1 = 0x0000;

		pRegs->CANIF1_ARB2 = LPC11xx_CAN_IFARB2_ID_MVAL;

		if (i > 1) {
			pRegs->CANIF1_MSK2 = LPC11xx_CAN_IFCMDMSK_MASK_MXTD;
			pRegs->CANIF1_ARB2 |= LPC11xx_CAN_IFARB2_ID_MTD;
		}

		if ( (i % 0x02) == 0 ) {
			pRegs->CANIF1_MCTRL = LPC11xx_CANIFMCTRL_UMSK | LPC11xx_CANIFMCTRL_TXIE | LPC11xx_CANIFMCTRL_EOB | 0x08;
			pRegs->CANIF1_MSK2 |= LPC11xx_CAN_IFCMDMSK_MASK_MDIR;
			pRegs->CANIF1_ARB2 |= LPC11xx_CAN_IFARB2_ID_DIR;
		}
		else {
			pRegs->CANIF1_MCTRL = LPC11xx_CANIFMCTRL_UMSK | LPC11xx_CANIFMCTRL_RXIE | LPC11xx_CANIFMCTRL_EOB | 0x08 ;
		}
		pRegs->CANIF1_DA1 = 0x0000;
		pRegs->CANIF1_DA2 = 0x0000;
		pRegs->CANIF1_DB1 = 0x0000;
		pRegs->CANIF1_DB2 = 0x0000;

		/* Transfer data to message RAM */
		pRegs->CANIF1_CMDREQ = i+1;
		while (pRegs->CANIF1_CMDREQ & LPC11xx_CAN_IFCREQ_BUSY );
	}
	return;
}


/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR canSetConfig(BT_HANDLE hCan, BT_CAN_CONFIG *pConfig) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hCan->pDevice, BT_RESOURCE_IRQ, 0);
	BT_DisableInterrupt(pResource->ulStart);

	BT_ERROR Error = BT_ERR_NONE;


	if(!hCan->hRxFifo && !hCan->hTxFifo) {
		hCan->hRxFifo = BT_FifoCreate(pConfig->usRxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);
		hCan->hTxFifo = BT_FifoCreate(pConfig->usTxBufferSize, sizeof(BT_CAN_MESSAGE), 0, &Error);
	}

#ifdef BT_CONFIG_MACH_USE_CAN_ROMDRIVER

	canSetBaudrate(hCan, pConfig->ulBaudrate);

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
#else
	volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;

	if ( !(pRegs->CANCNTL & LPC11xx_CAN_CTRL_INIT) ) {
		/* If it's in normal operation already, stop it, reconfigure
		everything first, then restart. */
		pRegs->CANCNTL |= LPC11xx_CAN_CTRL_INIT;		/* Default state */
	}

	canSetBaudrate(hCan, pConfig->ulBaudrate);


	/* Initialization finishes, normal operation now. */
	pRegs->CANCNTL &= ~LPC11xx_CAN_CTRL_INIT;
	while ( pRegs->CANCNTL & LPC11xx_CAN_CTRL_INIT );

	canConfigureMessages(hCan);

	/* By default, auto TX is enabled, enable all related interrupts */
	pRegs->CANCNTL |= (LPC11xx_CAN_CTRL_IE |
					   LPC11xx_CAN_CTRL_SIE |
					   LPC11xx_CAN_CTRL_EIE);

#endif

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

	BT_FifoRead(hCan->hRxFifo, 1, pCanMessage, 0);

	return Error;
}

/**
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR canWrite(BT_HANDLE hCan, BT_CAN_MESSAGE *pCanMessage) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_FifoWrite(hCan->hTxFifo, 1, pCanMessage, 0);

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

		BT_FifoRead(hCan->hTxFifo, 1, &oMessage, 0);
#ifdef BT_CONFIG_MACH_USE_CAN_ROMDRIVER
		oMsgObj.ucmsgobj = 2;
		oMsgObj.uclength = oMessage.ucLength;
		oMsgObj.ulmask   = 0x0000;
		oMsgObj.ulmodeid = oMessage.ulID & ~BT_CAN_EFF_FLAG;

		if (oMessage.ulID & BT_CAN_EFF_FLAG) {
			oMsgObj.ulmodeid |= LPC11xx_CAN_MSGOBJ_EXT;
		}

		memcpy(oMsgObj.ucdata, oMessage.ucdata, oMessage.ucLength);
		g_Msg_Queue[hCan->id] = 1;


		(*rom)->pCAND->can_transmit(&oMsgObj);
#else
		volatile LPC11xx_CAN_REGS *pRegs = hCan->pRegs;
		BT_u32 msg_no, tx_id;

		tx_id = oMessage.ulID & ~BT_CAN_EFF_FLAG;

		pRegs->CANIF1_MSK2 = 0;
		pRegs->CANIF1_MSK1 = 0;

		if (oMessage.ulID & BT_CAN_EFF_FLAG) {
			msg_no = 2;
			/* MsgVal: 1, Mtd: 1, Dir: 1, ID = 0x200000 */
			pRegs->CANIF1_ARB2 = LPC11xx_CAN_ID_MVAL | LPC11xx_CAN_ID_MTD | LPC11xx_CAN_ID_DIR | (tx_id >> 16);
			pRegs->CANIF1_ARB1 = tx_id & 0x0000FFFF;

		}
		else {		/* standard frame */
			msg_no = 0;
			/* MsgVal: 1, Mtd: 0, Dir: 1, ID = 0x200 */
			pRegs->CANIF1_ARB2 = LPC11xx_CAN_ID_MVAL | LPC11xx_CAN_ID_DIR | (tx_id << 2);
			pRegs->CANIF1_ARB1 = 0x0000;

		}

		pRegs->CANIF1_MCTRL = LPC11xx_CANIFMCTRL_UMSK |
							  LPC11xx_CANIFMCTRL_TXRQ |
							  LPC11xx_CANIFMCTRL_EOB |
							  (oMessage.ucLength & 0x0F);

		BT_u16 usBuffer[4];

		memcpy(usBuffer, oMessage.ucdata, 8);

		pRegs->CANIF1_DA1 = usBuffer[0];
		pRegs->CANIF1_DA2 = usBuffer[1];
		pRegs->CANIF1_DB1 = usBuffer[2];
		pRegs->CANIF1_DB2 = usBuffer[3];

		pRegs->CANIF1_CMDMSK = LPC11xx_CAN_IFCMDMSK_WR |
							   LPC11xx_CAN_IFCMDMSK_MASK |
							   LPC11xx_CAN_IFCMDMSK_ARB |
							   LPC11xx_CAN_IFCMDMSK_CTRL |
							   LPC11xx_CAN_IFCMDMSK_TREQ |
							   LPC11xx_CAN_IFCMDMSK_DATAA |
							   LPC11xx_CAN_IFCMDMSK_DATAB;
		pRegs->CANIF1_CMDREQ = msg_no+1;
		while( pRegs->CANIF1_CMDREQ & LPC11xx_CAN_IFCREQ_BUSY );   /* Could spin here forever */
#endif
	}
	return;
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
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = canCleanup,												///< Handle's cleanup routine.
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

	hCan->id = pResource->ulStart;

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
		.ulStart			= 29,
		.ulEnd				= 29,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC11xx_can0_device = {
	.name 					= "LPC11xx,can",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_can0_resources),
	.pResources 			= oLPC11xx_can0_resources,
};

const BT_DEVFS_INODE_DEF oLPC11xx_can0_inode = {
	.szpName = "can0",
	.pDevice = &oLPC11xx_can0_device,
};
#endif
