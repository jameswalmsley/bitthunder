/**
 *	Provides the Xilinx system timer for BitThunder.
 **/

#include <bitthunder.h>
#include "timer.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LM3Sxx-TIMER")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx Timers kernel driver, also providing kernel tick")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

struct _TIMER_CALLBACK_HANDLE {
	BT_HANDLE_HEADER	h;
	BT_TIMER_CALLBACK	pfnCallback;
	void 			   *pParam;
} ;

/**
 *	We can define how a handle should look in a Timer driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 					h;			///< All handles must include a handle header.
	LM3Sxx_TIMER_REGS				   *pRegs;
	const BT_INTEGRATED_DEVICE   	   *pDevice;
	struct _TIMER_CALLBACK_HANDLE	   *pCallback;
	BT_u32								ulId;
	BT_u16								uLastEventTime;
	BT_u16								uEventTime;
	BT_TIMER_MODE						eMode;
	BT_TIMER_CONFIG_MODE				eConfig;
};

static BT_HANDLE g_TIMER_HANDLES[8] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static void disableTimerPeripheralClock(BT_HANDLE hTimer);

static const BT_IF_HANDLE oCallbackHandleInterface;


BT_ERROR TMRA_Interrupt(BT_HANDLE hTimer) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	hTimer->pRegs->TMRICR = 0x000001F;

	if (hTimer->pCallback)
		hTimer->pCallback->pfnCallback(hTimer, hTimer->pCallback->pParam);

	hTimer->uLastEventTime = hTimer->uEventTime;
	hTimer->uEventTime		= pRegs->TMRTAR;

	return BT_ERR_NONE;
}

BT_ERROR TMRB_Interrupt(BT_HANDLE hTimer) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	hTimer->pRegs->TMRICR = 0x00000F00;

	if (hTimer->pCallback)
		hTimer->pCallback->pfnCallback(hTimer, hTimer->pCallback->pParam);

	hTimer->uLastEventTime	= hTimer->uEventTime;
	hTimer->uEventTime 		= pRegs->TMRTBR;

	return BT_ERR_NONE;
}

BT_ERROR BT_NVIC_IRQ_35(void) {
	return TMRA_Interrupt(g_TIMER_HANDLES[0]);
}

BT_ERROR BT_NVIC_IRQ_36(void) {
	return TMRB_Interrupt(g_TIMER_HANDLES[1]);
}

BT_ERROR BT_NVIC_IRQ_37(void) {
	return TMRA_Interrupt(g_TIMER_HANDLES[2]);
}

BT_ERROR BT_NVIC_IRQ_38(void) {
	return TMRB_Interrupt(g_TIMER_HANDLES[3]);
}

BT_ERROR BT_NVIC_IRQ_39(void) {
	return TMRA_Interrupt(g_TIMER_HANDLES[4]);
}

BT_ERROR BT_NVIC_IRQ_40(void) {
	return TMRB_Interrupt(g_TIMER_HANDLES[5]);
}

BT_ERROR BT_NVIC_IRQ_51(void) {
	return TMRA_Interrupt(g_TIMER_HANDLES[6]);
}

BT_ERROR BT_NVIC_IRQ_52(void) {
	return TMRB_Interrupt(g_TIMER_HANDLES[7]);
}


static void ResetTimer(BT_HANDLE hTimer) {
}


static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	ResetTimer(hTimer);

	// Disable peripheral clock.
	disableTimerPeripheralClock(hTimer);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	g_TIMER_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_u32 timer_getinputclock(BT_HANDLE hTimer, BT_ERROR *pError) {
	if (pError)
		*pError = BT_ERR_NONE;

	return BT_LM3Sxx_GetMainFrequency();
}

static BT_ERROR timer_setconfig(BT_HANDLE hTimer, BT_TIMER_CONFIG *pConfig) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	volatile BT_u32 *pTMRModeReg;

	if (pConfig->eConfig == BT_TIMER_CONFIG_32BIT_TMR)
		pRegs->TMRCFG = 0x00000000;
	else if (pConfig->eConfig == BT_TIMER_CONFIG_32BIT_RTC)
		pRegs->TMRCFG = 0x00000001;
	else if (pConfig->eConfig == BT_TIMER_CONFIG_16BIT_TMR)
		pRegs->TMRCFG = 0x00000004;

	if ((hTimer->ulId % 2) == 0) {
		pTMRModeReg = &(pRegs->TMRTAMR);
		pRegs->TMRCTL &= ~LM3Sxx_TIMER_TMRCTL_TMRA_EVENT_MASK;
		if (pConfig->eEdge == BT_TIMER_EDGE_POSITIVE)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRA_EVENT_POS_EDGE;
		else if (pConfig->eEdge == BT_TIMER_EDGE_NEGATIVE)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRA_EVENT_NEG_EDGE;
		else if (pConfig->eEdge == BT_TIMER_EDGE_BOTH)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRA_EVENT_BOTH_EDGES;
	}
	else {
		pTMRModeReg = &(pRegs->TMRTBMR);
		pRegs->TMRCTL &= ~LM3Sxx_TIMER_TMRCTL_TMRB_EVENT_MASK;
		if (pConfig->eEdge == BT_TIMER_EDGE_POSITIVE)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRB_EVENT_POS_EDGE;
		else if (pConfig->eEdge == BT_TIMER_EDGE_NEGATIVE)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRB_EVENT_NEG_EDGE;
		else if (pConfig->eEdge == BT_TIMER_EDGE_BOTH)
			pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRB_EVENT_BOTH_EDGES;
	}
	*pTMRModeReg = 0x00000000;
	if (pConfig->bWaitOnTrigger)
		*pTMRModeReg |= 0x00000040;
	if (pConfig->bInterruptOnMatch)
		*pTMRModeReg |= 0x00000020;
	if ((pConfig->eDirection == BT_TIMER_DIRECTION_UP) && (pConfig->eConfig != BT_TIMER_CONFIG_32BIT_RTC))
		*pTMRModeReg |= 0x00000010;

	if ((hTimer->ulId % 2) == 0) {
		pRegs->TMRIMR &= ~LM3Sxx_TIMER_TMRIMR_CAPTURE_A_EVENT;
	}
	else {
		pRegs->TMRIMR &= ~LM3Sxx_TIMER_TMRIMR_CAPTURE_B_EVENT;
	}

	hTimer->eMode = pConfig->eMode;
	hTimer->eConfig = pConfig->eConfig;

	switch (pConfig->eMode) {
	case BT_TIMER_MODE_ONESHOT: {
		*pTMRModeReg |= 0x00000001;
		break;
	}
	case BT_TIMER_MODE_PERIODIC: {
		*pTMRModeReg |= 0x00000002;
		break;
	}
	case BT_TIMER_MODE_CAPTURE_COUNT: {
		*pTMRModeReg |= 0x00000003;
		if ((hTimer->ulId % 2) == 0)
			pRegs->TMRTAMATCHR = 0x00000000;
		else
			pRegs->TMRTBMATCHR = 0x00000000;
		break;
	}
	case BT_TIMER_MODE_CAPTURE_TIME: {
		*pTMRModeReg |= 0x00000007;
		if ((hTimer->ulId % 2) == 0) {
			pRegs->TMRIMR |= LM3Sxx_TIMER_TMRIMR_CAPTURE_A_EVENT;
		}
		else {
			pRegs->TMRIMR |= LM3Sxx_TIMER_TMRIMR_CAPTURE_B_EVENT;
		}
		break;
	}
	case BT_TIMER_MODE_PWM: {
		*pTMRModeReg |= 0x0000000A;
		*pTMRModeReg &= ~0x00000010;
		break;
	}
	default: {
		break;
	}
	}

	return BT_ERR_NONE;
}

static BT_ERROR timer_getconfig(BT_HANDLE hTimer, void *pConfig) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	return BT_ERR_NONE;
}


static BT_ERROR timer_start(BT_HANDLE hTimer) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if ((hTimer->ulId % 2) == 0)
		pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRA_EN;
	else
		pRegs->TMRCTL |= LM3Sxx_TIMER_TMRCTL_TMRB_EN;

	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if ((hTimer->ulId % 2) == 0)
		pRegs->TMRCTL &= ~LM3Sxx_TIMER_TMRCTL_TMRA_EN;
	else
		pRegs->TMRCTL &= ~LM3Sxx_TIMER_TMRCTL_TMRB_EN;

	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_ERROR timer_callback_cleanup(BT_HANDLE hCallback) {
	struct _TIMER_CALLBACK_HANDLE *hCleanup = (struct _TIMER_CALLBACK_HANDLE*)hCallback;

	hCleanup->pfnCallback = NULL;
	hCleanup->pParam	  = NULL;

	BT_CloseHandle((BT_HANDLE)hCleanup);

	return BT_ERR_NONE;
}

static BT_HANDLE timer_register_callback(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	struct _TIMER_CALLBACK_HANDLE *pCallback = (struct _TIMER_CALLBACK_HANDLE*)BT_CreateHandle(&oCallbackHandleInterface, sizeof(struct _TIMER_CALLBACK_HANDLE),pError);

	if (pCallback) {
		pCallback->pfnCallback = pfnCallback;
		pCallback->pParam	   = pParam;

		hTimer->pCallback      = pCallback;
	}

	return (BT_HANDLE)pCallback;
}

static BT_ERROR timer_unregister_callback(BT_HANDLE hTimer, BT_HANDLE hCallback) {
	timer_callback_cleanup(hCallback);

	hTimer->pCallback = NULL;

	return BT_ERR_NONE;
}


static BT_u32 timer_get_prescaler(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError = BT_ERR_NONE;

	if ((hTimer->ulId % 2) == 0)
		return pRegs->TMRTAPR + 1;
	else
		return pRegs->TMRTBPR + 1;
	return 1;
}

static BT_ERROR timer_set_prescaler(BT_HANDLE hTimer, BT_u32 ulPrescaler) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if ((hTimer->ulId % 2) == 0)
		pRegs->TMRTAPR = ulPrescaler - 1;
	else
		pRegs->TMRTBPR = ulPrescaler - 1;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_period_count(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	if ((hTimer->ulId % 2) == 0)
		return (pRegs->TMRTAILR + 1) * (pRegs->TMRTAPR + 1);
	else
		return (pRegs->TMRTBILR + 1) * (pRegs->TMRTBPR + 1);
}

static BT_ERROR timer_set_period_count(BT_HANDLE hTimer, BT_u32 ulValue) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	BT_u32 Err, a, b;
	if (hTimer->eConfig == BT_TIMER_CONFIG_16BIT_TMR) {
		BT_u32 i, j;
		Err = 0xFFFFFFFF;

		for (i = 0; i < 256; i++) {
			BT_u32 ulSub = ulValue / (i+1);
			if (ulSub >= 65536)
				continue;
			BT_u32 ulAbs = ulValue - ulSub * (i+1);
			if (ulAbs < Err) {
				Err = ulAbs;
				a = i;
				b = ulSub;
			}
		}
	}
	else {
		a = 0;
		b = ulValue;
	}

	if ((hTimer->ulId % 2) == 0) {
		pRegs->TMRTAILR = b - 1;
		pRegs->TMRTAPR = a;
	}
	else {
		pRegs->TMRTBILR = b - 1;
		pRegs->TMRTBPR = a;
	}

	return BT_ERR_NONE;
}

static BT_u32 timer_get_match(BT_HANDLE hTimer, BT_u32 ulChannel, BT_ERROR *pError) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	if ((hTimer->ulId % 2) == 0)
		return pRegs->TMRTAMATCHR;
	else
		return pRegs->TMRTBMATCHR;
}

static BT_ERROR timer_set_match(BT_HANDLE hTimer, BT_u32 ulChannel, BT_u32 ulValue) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	BT_u32 Err, a, b;
	if (hTimer->eConfig == BT_TIMER_CONFIG_16BIT_TMR) {
		BT_u32 i, j;
		Err = 0xFFFFFFFF;

		for (i = 0; i < 256; i++) {
			BT_u32 ulSub = ulValue / i;
			if (ulSub >= 65536)
				continue;
			BT_u32 ulAbs = ulValue - ulSub * i;
			if (ulAbs < Err) {
				Err = ulAbs;
				a = i;
				b = ulSub;
			}
		}
	}
	else {
		a = 0;
		b = ulValue;
	}

	if ((hTimer->ulId % 2) == 0) {
		pRegs->TMRTAMATCHR = b;
		pRegs->TMRTAPMR = a;
	}
	else {
		pRegs->TMRTBMATCHR = b;
		pRegs->TMRTBPMR = a;
	}

	return BT_ERR_NONE;
}


static BT_ERROR timer_enable_reload(BT_HANDLE hTimer) {

	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_disable_reload(BT_HANDLE hTimer) {

	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_getvalue(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	if (hTimer->eMode == BT_TIMER_MODE_CAPTURE_TIME) {
		BT_u16 uResult = (hTimer->uLastEventTime - hTimer->uEventTime);
		return uResult;
	}
	else {
		if ((hTimer->ulId % 2) == 0)
			return pRegs->TMRTAR;
		else
			return pRegs->TMRTBR;
	}
}

static BT_ERROR timer_setvalue(BT_HANDLE hTimer, BT_u32 ulValue) {
	volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if ((hTimer->ulId % 2) == 0)
		pRegs->TMRTAR = ulValue;
	else
		pRegs->TMRTBR = ulValue;

	return BT_ERR_NONE;
}

/**
 *	This actually allows the TimerS to be clocked!
 **/
static void enableTimerPeripheralClock(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER0EN;
		break;
	}
	case 2: case 3:{
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER1EN;
		break;
	}
	case 4: case 5: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER2EN;
		break;
	}
	case 6: case 7: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER3EN;
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
static void disableTimerPeripheralClock(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if (g_TIMER_HANDLES[1] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER0EN;
		break;
	}
	case 1: {
		if (g_TIMER_HANDLES[0] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER0EN;
		break;
	}
	case 2: {
		if (g_TIMER_HANDLES[3] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER1EN;
		break;
	}
	case 3: {
		if (g_TIMER_HANDLES[2] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER1EN;
		break;
	}
	case 4: {
		if (g_TIMER_HANDLES[5] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER2EN;
		break;
	}
	case 5: {
		if (g_TIMER_HANDLES[4] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER2EN;
		break;
	}
	case 6: {
		if (g_TIMER_HANDLES[7] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER3EN;
		break;
	}
	case 7: {
		if (g_TIMER_HANDLES[6] == NULL) LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER3EN;
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
static BT_BOOL isTimerPeripheralClockEnabled(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 2: case 3: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER1EN) {
			return BT_TRUE;
		}
		break;
	}
	case 4: case 5: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER2EN) {
			return BT_TRUE;
		}
		break;
	}
	case 6: case 7: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER3EN) {
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
 *	This implements the Timer power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR TimerSetPowerState(BT_HANDLE hTimer, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableTimerPeripheralClock(hTimer);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableTimerPeripheralClock(hTimer);
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
 *	This implements the Timer power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR TimerGetPowerState(BT_HANDLE hTimer, BT_POWER_STATE *pePowerState) {
	if(isTimerPeripheralClockEnabled(hTimer)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_TIMER oTimerDeviceInterface= {
	.pfnGetInputClock			= timer_getinputclock,
	.pfnSetConfig				= timer_setconfig,
	.pfnGetConfig				= timer_getconfig,
	.pfnStart					= timer_start,
	.pfnStop					= timer_stop,
	.pfnEnableInterrupt			= timer_enable_interrupt,
	.pfnDisableInterrupt		= timer_disable_interrupt,
	.pfnRegisterCallback		= timer_register_callback,
	.pfnUnregisterCallback		= timer_unregister_callback,
	.pfnGetPrescaler			= timer_get_prescaler,
	.pfnSetPrescaler			= timer_set_prescaler,
	.pfnGetPeriodCount			= timer_get_period_count,
	.pfnSetPeriodCount			= timer_set_period_count,
	.pfnGetMatch				= timer_get_match,
	.pfnSetMatch				= timer_set_match,
	.pfnEnableReload			= timer_enable_reload,
	.pfnDisableReload			= timer_disable_reload,
	.pfnGetValue				= timer_getvalue,
	.pfnSetValue				= timer_setvalue,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oTimerDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	TimerSetPowerState,											///< Pointers to the power state API implementations.
	TimerGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LM3Sxx_TIMER_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_TIMER,											///< Allow configuration through the Timer api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oTimerDeviceInterface,
	},
};

static const BT_IF_HANDLE oCallbackHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType		= BT_HANDLE_T_CALLBACK,											///< Handle Type!
	.pfnCleanup = timer_callback_cleanup,												///< Handle's cleanup routine.
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_TIMER_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = timer_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE timer_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hTimer = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_TIMER_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hTimer = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hTimer) {
		goto err_out;
	}

	g_TIMER_HANDLES[pResource->ulStart] = hTimer;

	hTimer->pDevice = pDevice;

	hTimer->ulId = pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hTimer->pRegs = (LM3Sxx_TIMER_REGS *) pResource->ulStart;

	TimerSetPowerState(hTimer, BT_POWER_STATE_AWAKE);

	ResetTimer(hTimer);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, timer_irq_handler, hTimer);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hTimer;

err_free_out:
	BT_kFree(hTimer);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF timer_driver = {
	.name 		= "LM3Sxx,timer",
	.pfnProbe	= timer_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_0_A
static const BT_RESOURCE oLM3Sxx_timer0_A_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 35,
		.ulEnd				= 35,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer0_A_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer0_A_resources),
	.pResources 			= oLM3Sxx_timer0_A_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer0_A_inode = {
	.szpName = "timer0A",
	.pDevice = &oLM3Sxx_timer0_A_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_0_B
static const BT_RESOURCE oLM3Sxx_timer0_B_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 36,
		.ulEnd				= 36,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer0_B_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer0_B_resources),
	.pResources 			= oLM3Sxx_timer0_B_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer0_B_inode = {
	.szpName = "timer0B",
	.pDevice = &oLM3Sxx_timer0_B_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_1_A
static const BT_RESOURCE oLM3Sxx_timer1_A_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 2,
		.ulEnd				= 2,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 37,
		.ulEnd				= 37,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer1_A_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer1_A_resources),
	.pResources 			= oLM3Sxx_timer1_A_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer1_A_inode = {
	.szpName = "timer1A",
	.pDevice = &oLM3Sxx_timer1_A_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_1_B
static const BT_RESOURCE oLM3Sxx_timer1_B_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 3,
		.ulEnd				= 3,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 38,
		.ulEnd				= 38,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer1_B_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer1_B_resources),
	.pResources 			= oLM3Sxx_timer1_B_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer1_B_inode = {
	.szpName = "timer1B",
	.pDevice = &oLM3Sxx_timer1_B_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_2_A
static const BT_RESOURCE oLM3Sxx_timer2_A_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 4,
		.ulEnd				= 4,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 39,
		.ulEnd				= 39,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer2_A_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer2_A_resources),
	.pResources 			= oLM3Sxx_timer2_A_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer2_A_inode = {
	.szpName = "timer2A",
	.pDevice = &oLM3Sxx_timer2_A_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_2_B
static const BT_RESOURCE oLM3Sxx_timer2_B_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 5,
		.ulEnd				= 5,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 40,
		.ulEnd				= 40,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer2_B_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer2_B_resources),
	.pResources 			= oLM3Sxx_timer2_B_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer2_B_inode = {
	.szpName = "timer2B",
	.pDevice = &oLM3Sxx_timer2_B_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_3_A
static const BT_RESOURCE oLM3Sxx_timer3_A_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 6,
		.ulEnd				= 6,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 51,
		.ulEnd				= 51,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer3_A_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer3_A_resources),
	.pResources 			= oLM3Sxx_timer3_A_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer3_A_inode = {
	.szpName = "timer3A",
	.pDevice = &oLM3Sxx_timer3_A_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_3_B
static const BT_RESOURCE oLM3Sxx_timer3_B_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 7,
		.ulEnd				= 7,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 52,
		.ulEnd				= 52,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer3_B_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer3_B_resources),
	.pResources 			= oLM3Sxx_timer3_B_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer3_B_inode = {
	.szpName = "timer3B",
	.pDevice = &oLM3Sxx_timer3_B_device,
};
#endif
