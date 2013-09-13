/**
 *	Provides the LPC11xx adc for BitThunder.
 **/

#include <bitthunder.h>
#include "adc.h"
#include "rcc.h"
#include <collections/bt_fifo.h>


BT_DEF_MODULE_NAME			("LPC11xx-ADC")
BT_DEF_MODULE_DESCRIPTION	("LPC11xx adc kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a ADC driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;				///< All handles must include a handle header.
	LPC11xx_ADC_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_ADC_OPERATING_MODE	eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_u32					ulSWAverageCount;
	BT_HANDLE		   		hFifo[8];		///< fifo - ring buffer for every data channel.
};

static BT_HANDLE g_ADC_HANDLES[1] = {
	NULL,
};

static void disableAdcPeripheralClock(BT_HANDLE hAdc);


BT_ERROR BT_NVIC_IRQ_40(void) {
	BT_HANDLE hAdc = g_ADC_HANDLES[0];

	volatile LPC11xx_ADC_REGS *pRegs = hAdc->pRegs;
	BT_ERROR Error;

	pRegs->ADCR &= ~LPC11xx_ADC_ADCR_BURST;

	static BT_u32 ulSum[8] = {0};
	static BT_u32 ulAverage;

	BT_u32 ulActive = pRegs->ADSTAT & 0x000000FF;		/* Read ADC will clear the interrupt */

	BT_u32 i;
	for (i = 0; i < 8; i++) {
		if (ulActive & (0x01<<i))
			ulSum[i] += ((pRegs->ADData[i] >> 6) & 0x3FF);
	}
	if (ulActive) ulAverage++;
	if (ulAverage >= hAdc->ulSWAverageCount) {
		for (i = 0; i < 8; i++) {
			if (ulActive & (0x01<<i)) {
				ulSum[i] /= ulAverage;
				BT_FifoWrite(hAdc->hFifo[i], 1, &ulSum[i], &Error);
				ulSum[i] = 0;
			}
		}
		ulAverage = 0;
	}

	pRegs->ADCR |= LPC11xx_ADC_ADCR_BURST;

	return Error;
}

static BT_ERROR adc_cleanup(BT_HANDLE hAdc) {

	// Disable peripheral clock.
	disableAdcPeripheralClock(hAdc);

	// Free any buffers if used.
	BT_u32 i;
	for (i = 0; i < 8; i++) {
		BT_CloseHandle(hAdc->hFifo[i]);
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	g_ADC_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_ERROR adc_setconfig(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	volatile LPC11xx_ADC_REGS *pRegs = hAdc->pRegs;
	BT_ERROR Error = BT_ERR_NONE;

	pRegs->ADCR &= ~(0x000E0000);
	if ((pConfig->ulResolution >= 3) && (pConfig->ulResolution <= 10)) {
		pRegs->ADCR |= (10 - pConfig->ulResolution) << 17;
	}

	BT_u32 ulClocksPerConversion = 11 - ((pRegs->ADCR >> 17) & 0x7);

	BT_u32 ulInputClk = BT_LPC11xx_GetMainFrequency();
	BT_u32 ulDivider  = ulInputClk / (pConfig->ulSampleRate * ulClocksPerConversion);

	if (ulDivider == 0) ulDivider++;

	while ((ulInputClk / ulDivider) > LPC11xx_ADC_MAX_CLOCK) ulDivider++;

	pRegs->ADCR &= ~(0x0000FF00);
	pRegs->ADCR |= (ulDivider - 1) << 8;

	hAdc->ulSWAverageCount = pConfig->ulSWAverageCount;

	pRegs->ADINTEN &= ~LPC11xx_ADC_ADINTEN_ADGINTEN;	// Disable the interrupt

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hAdc->eMode !=  BT_ADC_MODE_POLLED) {

			BT_u32 i;
			for (i = 0; i < 8; i++) {
				BT_CloseHandle(hAdc->hFifo[i]);
			}

			// Disable TX and RX interrupts
			hAdc->eMode = BT_ADC_MODE_POLLED;
		}

		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		pRegs->ADCR &= ~(0x000000FF);
		pRegs->ADCR	   |= pConfig->ulActiveChannels;
		if(hAdc->eMode != BT_ADC_MODE_BUFFERED) {
			BT_u32 i;
			for (i = 0; i < 8; i++) {
				if(!hAdc->hFifo[i]) {
					hAdc->hFifo[i] = BT_FifoCreate(pConfig->ulBufferSize, 4, BT_FIFO_OVERWRITE, &Error);
				}
			}

			i = 0;
			while (pConfig->ulActiveChannels & (0x1<<i)) {
				pRegs->ADINTEN = 0x1<<i;	// Enable the interrupt
				i++;
			}

			hAdc->eMode = BT_ADC_MODE_BUFFERED;
		}
		break;
	}

	default:
		// Unsupported operating mode!
		break;
	}

	return BT_ERR_NONE;
}

static BT_ERROR adc_getconfig(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;

	pConfig->ulBufferSize	= BT_FifoSize(hAdc->hFifo[0], &Error);
	pConfig->eMode			= hAdc->eMode;

	return BT_ERR_NONE;
}

static void adc_enable(void) {
	volatile LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	pRCC->PDRUNCFG &= ~LPC11xx_RCC_PDRUNCFG_ADC_PD;

}

static BT_ERROR adc_start(BT_HANDLE hAdc) {
	volatile LPC11xx_ADC_REGS *pRegs = hAdc->pRegs;

	if (hAdc->eMode == BT_ADC_MODE_BUFFERED) {
		pRegs->ADCR |= LPC11xx_ADC_ADCR_BURST;
	}

	return BT_ERR_NONE;
}

static BT_ERROR adc_stop(BT_HANDLE hAdc) {
	volatile LPC11xx_ADC_REGS *pRegs = hAdc->pRegs;

	if (hAdc->eMode == BT_ADC_MODE_BUFFERED) {
		pRegs->ADCR &= ~LPC11xx_ADC_ADCR_BURST;
	}

	return BT_ERR_NONE;
}

static BT_ERROR adc_Read(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest) {
	volatile LPC11xx_ADC_REGS *pRegs = hAdc->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hAdc->eMode) {
	case BT_ADC_MODE_POLLED:
	{
		while(ulSize) {
			BT_u32 i;
			BT_u32 ulSum = 0;


			for (i = 0; i < hAdc->ulSWAverageCount; i++) {
				pRegs->ADCR &= 0xFFFFFF00;
				//pRegs->ADCR |= ;
				pRegs->ADCR |= LPC11xx_ADC_ADCR_START_NOW | (LPC11xx_ADC_ADCR_CHANNELSEL << ulChannel);

				while((pRegs->ADGDR & LPC11xx_ADC_ADGDR_DONE)) {
					BT_ThreadYield();
				}
				ulSum += (pRegs->ADGDR >> 6) & 0x3FF;

				pRegs->ADCR &= ~LPC11xx_ADC_ADCR_START_NOW;

			}

			*pucDest++ = ulSum / hAdc->ulSWAverageCount;
			ulSize--;
		}

		break;
	}

	case BT_ADC_MODE_BUFFERED:
	{
		// Get bytes from RX buffer very quickly.
		BT_FifoRead(hAdc->hFifo[ulChannel], ulSize, pucDest, &Error);
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;
	}
	return BT_ERR_NONE;
}

/**
 *	This actually allows the adc'S to be clocked!
 **/
static void enableAdcPeripheralClock(BT_HANDLE hAdc) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC11xx_RCC->SYSAHBCLKCTRL |= LPC11xx_RCC_SYSAHBCLKCTRL_ADCEN;
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
static void disableAdcPeripheralClock(BT_HANDLE hAdc) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~LPC11xx_RCC_SYSAHBCLKCTRL_ADCEN;
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
static BT_BOOL isAdcPeripheralClockEnabled(BT_HANDLE hAdc) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LPC11xx_RCC->SYSAHBCLKCTRL & LPC11xx_RCC_SYSAHBCLKCTRL_ADCEN) {
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
 *	This implements the ADC power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR AdcSetPowerState(BT_HANDLE hAdc, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableAdcPeripheralClock(hAdc);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableAdcPeripheralClock(hAdc);
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
 *	This implements the ADC power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR AdcGetPowerState(BT_HANDLE hAdc, BT_POWER_STATE *pePowerState) {
	if(isAdcPeripheralClockEnabled(hAdc)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_ADC oAdcDeviceInterface= {
	.pfnSetConfig 		= adc_setconfig,
	.pfnGetConfig		= adc_getconfig,
	.pfnStart			= adc_start,
	.pfnStop			= adc_stop,
	.pfnRead			= adc_Read,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oAdcDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	AdcSetPowerState,											///< Pointers to the power state API implementations.
	AdcGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LPC11xx_ADC_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_ADC,											///< Allow configuration through the ADC api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oAdcDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC11xx_ADC_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = adc_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE adc_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hAdc = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_ADC_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hAdc = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hAdc) {
		goto err_out;
	}

	g_ADC_HANDLES[pResource->ulStart] = hAdc;

	hAdc->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hAdc->pRegs = (LPC11xx_ADC_REGS *) pResource->ulStart;

	AdcSetPowerState(hAdc, BT_POWER_STATE_AWAKE);

	adc_enable();

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, adc_irq_handler, hAdc);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hAdc;

err_free_out:
	BT_DestroyHandle(hAdc);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF adc_driver = {
	.name 		= "LPC11xx,adc",
	.pfnProbe	= adc_probe,
};


#ifdef BT_CONFIG_MACH_LPC11xx_ADC_0
static const BT_RESOURCE oLPC11xx_adc0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC11xx_ADC0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC11xx_ADC0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 40,
		.ulEnd				= 40,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC11xx_adc0_device = {
	.name 					= "LPC11xx,adc",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_adc0_resources),
	.pResources 			= oLPC11xx_adc0_resources,
};

const BT_DEVFS_INODE_DEF oLPC11xx_adc0_inode = {
	.szpName = "adc0",
	.pDevice = &oLPC11xx_adc0_device,
};
#endif
