/**
 *	Provides the LPC17xx  pwm for BitThunder.
 **/

#include <bitthunder.h>
#include "pwm.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LPC17xx-PWM")
BT_DEF_MODULE_DESCRIPTION	("LPC17xx pwm kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a PWM driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC17xx_PWM_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_u32					id;
};

static BT_HANDLE g_PWM_HANDLES[1] = {
	NULL,
};

static const BT_u32 g_PWM_PERIPHERAL[1] = {6};

static void disablePwmPeripheralClock(BT_HANDLE hPwm);



BT_ERROR BT_NVIC_IRQ_25(void) {
	return 0;
}

static void ResetPwm(BT_HANDLE hPwm)
{
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	pRegs->PWMIR	= 0;
	pRegs->PWMTCR	= 0;
	pRegs->PWMTC	= 0;
	pRegs->PWMPR	= 0;
	pRegs->PWMPC	= 0;
	pRegs->PWMMCR	= 0;
	pRegs->PWMMR0	= 0;
	pRegs->PWMMR1	= 0;
	pRegs->PWMMR2	= 0;
	pRegs->PWMMR3	= 0;
	pRegs->PWMMR4	= 0;
	pRegs->PWMMR5	= 0;
	pRegs->PWMMR6	= 0;
	pRegs->PWMCCR	= 0;
	pRegs->PWMCR0	= 0;
	pRegs->PWMCR1	= 0;
	pRegs->PWMCR2	= 0;
	pRegs->PWMCR3	= 0;
	pRegs->PWMCTCR	= 0;
}

static BT_ERROR pwm_cleanup(BT_HANDLE hPwm) {
	ResetPwm(hPwm);

	// Disable peripheral clock.
	disablePwmPeripheralClock(hPwm);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hPwm->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hPwm->pDevice, BT_RESOURCE_ENUM, 0);

	g_PWM_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_ERROR pwm_start(BT_HANDLE hPwm) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	pRegs->PWMTCR |= LPC17xx_PWM_PWMTCR_CEn;
	pRegs->PWMTCR |= LPC17xx_PWM_PWMTCR_PWMEn;

	return BT_ERR_NONE;
}

static BT_ERROR pwm_stop(BT_HANDLE hPwm) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	pRegs->PWMTCR &= ~LPC17xx_PWM_PWMTCR_CEn;
	pRegs->PWMTCR &= ~LPC17xx_PWM_PWMTCR_PWMEn;

	return BT_ERR_NONE;
}

static BT_u32 pwm_get_period_count(BT_HANDLE hPwm, BT_ERROR *pError) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	*pError= BT_ERR_NONE;

	return pRegs->PWMMR0;
}

static BT_ERROR pwm_set_frequency(BT_HANDLE hPwm, BT_u32 ulValue) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 ulInputClk = BT_LPC17xx_GetPeripheralClock(g_PWM_PERIPHERAL[hPwm->id]);
	BT_u32 ulPeriodCount = ulInputClk / ulValue;

	pRegs->PWMMR0 = ulPeriodCount;
	pRegs->PWMMCR |= LPC17xx_PWM_PWMMCR_MR0R;

	return BT_ERR_NONE;
}

static BT_u32 pwm_get_frequency(BT_HANDLE hPwm, BT_ERROR *pError) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	BT_u32 ulInputClk = BT_LPC17xx_GetPeripheralClock(g_PWM_PERIPHERAL[hPwm->id]) / (pRegs->PWMPR + 1);

	return ulInputClk / pRegs->PWMMR0;
}


static BT_u32 pwm_getdutycycle(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;

	*pError= BT_ERR_NONE;

	switch (ulChannel) {
	case 0: {
		return pRegs->PWMMR1;
	}
	case 1: {
		return pRegs->PWMMR2;
	}
	case 2: {
		return pRegs->PWMMR3;
	}
	case 3: {
		return pRegs->PWMMR4;
	}
	case 4: {
		return pRegs->PWMMR5;
	}
	case 5: {
		return pRegs->PWMMR6;
	}
	default: {
		break;
	}
	}
	return 0;
}

static BT_ERROR pwm_setdutycycle(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue) {
	volatile LPC17xx_PWM_REGS *pRegs = hPwm->pRegs;
	BT_BOOL bUpdate = BT_FALSE;

	switch (ulChannel) {
	case 0: {
		if (pRegs->PWMMR1 != ulValue) { pRegs->PWMMR1 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	case 1: {
		if (pRegs->PWMMR2 != ulValue) { pRegs->PWMMR2 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	case 2: {
		if (pRegs->PWMMR3 != ulValue) { pRegs->PWMMR3 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	case 3: {
		if (pRegs->PWMMR4 != ulValue) { pRegs->PWMMR4 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	case 4: {
		if (pRegs->PWMMR5 != ulValue) { pRegs->PWMMR5 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	case 5: {
		if (pRegs->PWMMR6 != ulValue) { pRegs->PWMMR6 = ulValue; bUpdate = BT_TRUE; }
		break;
	}
	default: {
		break;
	}
	}
	if (bUpdate) pRegs->PWMLER |= LPC17xx_PWM_PWMLER_ENA1 << ulChannel;
	if (!(pRegs->PWMPCR & (LPC17xx_PWM_PWMPCR_ENA1 << ulChannel))) pRegs->PWMPCR |= LPC17xx_PWM_PWMPCR_ENA1 << ulChannel;

	return BT_ERR_NONE;
}


static BT_ERROR pwm_setconfig(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig) {

	pwm_set_frequency(hPwm, pConfig->ulFrequency);
	return BT_ERR_NONE;
}

static BT_ERROR pwm_getconfig(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig) {

	return BT_ERR_NONE;
}

/**
 *	This actually allows the pwm'S to be clocked!
 **/
static void enablePwmPeripheralClock(BT_HANDLE hPwm) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hPwm->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_PWM0EN;
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
static void disablePwmPeripheralClock(BT_HANDLE hPwm) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hPwm->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_PWM0EN;
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
static BT_BOOL isPwmPeripheralClockEnabled(BT_HANDLE hPwm) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hPwm->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_PWM0EN) {
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
 *	This implements the PWM power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR PwmSetPowerState(BT_HANDLE hPwm, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disablePwmPeripheralClock(hPwm);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enablePwmPeripheralClock(hPwm);
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
 *	This implements the PWM power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR PwmGetPowerState(BT_HANDLE hPwm, BT_POWER_STATE *pePowerState) {
	if(isPwmPeripheralClockEnabled(hPwm)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_PWM oPwmDeviceInterface= {
	.pfnSetConfig 		= pwm_setconfig,
	.pfnGetConfig		= pwm_getconfig,
	.pfnStart			= pwm_start,
	.pfnStop			= pwm_stop,
	.pfnGetPeriodCount	= pwm_get_period_count,
	.pfnSetFrequency	= pwm_set_frequency,
	.pfnGetFrequency	= pwm_get_frequency,
	.pfnGetDutyCycle	= pwm_getdutycycle,
	.pfnSetDutyCycle	= pwm_setdutycycle,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oPwmDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	PwmSetPowerState,											///< Pointers to the power state API implementations.
	PwmGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LPC17xx_PWM_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_PWM,											///< Allow configuration through the PWM api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oPwmDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC17xx_PWM_oDeviceInterface,
	},
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	pwm_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE pwm_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hPwm = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_PWM_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hPwm = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hPwm) {
		goto err_out;
	}

	hPwm->id = pResource->ulStart;

	g_PWM_HANDLES[pResource->ulStart] = hPwm;

	hPwm->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hPwm->pRegs = (LPC17xx_PWM_REGS *) pResource->ulStart;

	PwmSetPowerState(hPwm, BT_POWER_STATE_AWAKE);

	ResetPwm(hPwm);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, pwm_irq_handler, hPwm);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hPwm;

err_free_out:
	BT_DestroyHandle(hPwm);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF pwm_driver = {
	.name 		= "LPC17xx,pwm",
	.pfnProbe	= pwm_probe,
};


#ifdef BT_CONFIG_MACH_LPC17xx_PWM_0
static const BT_RESOURCE oLPC17xx_pwm0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_PWM0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_PWM0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 25,
		.ulEnd				= 25,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_pwm0_device = {
	.name 					= "LPC17xx,pwm",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_pwm0_resources),
	.pResources 			= oLPC17xx_pwm0_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_pwm0_inode = {
	.szpName = "pwm0",
	.pDevice = &oLPC17xx_pwm0_device,
};
#endif
