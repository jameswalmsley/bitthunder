/**
 *	Provides the LM3Sxx adc for BitThunder.
 **/

#include <bitthunder.h>
#include "adc.h"
#include "rcc.h"
#include <collections/bt_fifo.h>


BT_DEF_MODULE_NAME			("LM3Sxx-ADC")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx adc kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a ADC driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;				///< All handles must include a handle header.
	LM3Sxx_ADC_REGS			   *pRegs;
	const BT_INTEGRATED_DEVICE *pDevice;
	BT_ADC_OPERATING_MODE		eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_u32						ulSWAverageCount;
	BT_HANDLE		   			hFifo[8];		///< fifo - ring buffer for every data channel.
};

static BT_HANDLE g_ADC_HANDLES[2] = {
	NULL,
	NULL,
};

static void disableAdcPeripheralClock(BT_HANDLE hAdc);


BT_ERROR BT_NVIC_IRQ_38(void) {
	//BT_HANDLE hAdc = g_ADC_HANDLES[0];

	//volatile LM3Sxx_ADC_REGS *pRegs = hAdc->pRegs;
	BT_ERROR Error;

	/*static BT_u32 ulSum[8] = {0};
	static BT_u32 ulAverage;

	//BT_u32 ulActive = pRegs->ADSTAT & 0x000000FF;		// Read ADC will clear the interrupt

	BT_u32 i;
	for (i = 0; i < 8; i++) {
		if (ulActive & (0x01<<i))
			ulSum[i] += ((pRegs->ADData[i] >> 4) & 0xFFF);		
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
	}*/

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
	volatile LM3Sxx_ADC_REGS *pRegs = hAdc->pRegs;
	BT_u32 i, j;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	BT_s32 slDivider  = (pConfig->ulSampleRate / 125000) - 1;

	if (slDivider < 0) slDivider = 0;
	if (slDivider > 3) slDivider = 3;

	BT_u32 ulShift = (pResource->ulStart + 4) * 2;
	BT_u32 ulMask  = ~(0x3 << ulShift);

	LM3Sxx_RCC->RCGC[0] = (LM3Sxx_RCC->RCGC[0] & ulMask) | (slDivider << ulShift);

	BT_u32 ulAvg = pConfig->ulHWAverageCount;
	if (ulAvg > 64) ulAvg = 64;
	while (ulAvg) {
		pRegs->ADCSAC++;
		ulAvg >>= 1;
	}

	pRegs->ADCCTL = (!pConfig->bUseIntReference) & 0x00000001;

	pRegs->ADCACTSS = 0x0000000F;		// Enable all sequencers

	for (i = 0; i < 3; i++) {
		pRegs->Sequencer[i].SSMUX = 0;
		pRegs->Sequencer[i].SSCTL = 0;
		if (pConfig->eInputMode == BT_ADC_INPUT_MODE_DIFFERENTIAL) {
			for (j = 0; j < 8; j++)
				pRegs->Sequencer[i].SSCTL |= (LM3Sxx_ADC_MUX_SSCTL_DIFF << j*4);
		}
	}
	pRegs->Sequencer[0].SSCTL = LM3Sxx_ADC_MUX_SSCTL_END << 28;
	pRegs->Sequencer[1].SSCTL = LM3Sxx_ADC_MUX_SSCTL_END << 16;
	pRegs->Sequencer[2].SSCTL = LM3Sxx_ADC_MUX_SSCTL_END << 16;
	pRegs->Sequencer[3].SSCTL = LM3Sxx_ADC_MUX_SSCTL_END << 4;

	hAdc->ulSWAverageCount = pConfig->ulSWAverageCount;

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hAdc->eMode !=  BT_ADC_MODE_POLLED) {

			BT_u32 i;
			for (i = 0; i < 8; i++) {
				BT_CloseHandle(hAdc->hFifo[i]);
			}

			hAdc->eMode = BT_ADC_MODE_POLLED;
		}

		//pRegs->ADINTEN &= ~LM3Sxx_ADC_ADINTEN_ADGINTEN;	// Disable the interrupt

		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		/*pRegs->ADCR &= ~(0x000000FF);
		pRegs->ADCR	   |= pConfig->ulActiveChannels;
		if(hAdc->eMode != BT_ADC_MODE_BUFFERED) {
			BT_u32 i;
			for (i = 0; i < 8; i++) {
				if(!hAdc->hFifo[i]) {
					hAdc->hFifo[i] = BT_FifoCreate(pConfig->ulBufferSize, 4, BT_FIFO_NONBLOCKING, &Error);
				}
			}
			//pRegs->ADINTEN |= LM3Sxx_ADC_ADINTEN_ADGINTEN;	// Enable the interrupt
			for (i = 0; i < 8; i++) {
				if (pConfig->ulActiveChannels & (0x1<<i)) {
					pRegs->ADINTEN = 0x1<<i;	// Enable the interrupt
				}
			}

			hAdc->eMode = BT_ADC_MODE_BUFFERED;
		}*/
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

	pConfig->ulBufferSize	= BT_FifoSize(hAdc->hFifo[0]);
	pConfig->eMode			= hAdc->eMode;

	return BT_ERR_NONE;
}


static BT_ERROR adc_start(BT_HANDLE hAdc) {
	/*volatile LM3Sxx_ADC_REGS *pRegs = hAdc->pRegs;

	pRegs->ADCR |= LM3Sxx_ADC_ADCR_PDN;

	if (hAdc->eMode == BT_ADC_MODE_BUFFERED) {
		pRegs->ADCR |= LM3Sxx_ADC_ADCR_BURST;
	}*/

	return BT_ERR_NONE;
}

static BT_ERROR adc_stop(BT_HANDLE hAdc) {
	/*volatile LM3Sxx_ADC_REGS *pRegs = hAdc->pRegs;

	pRegs->ADCR &= ~LM3Sxx_ADC_ADCR_PDN;

	if (hAdc->eMode == BT_ADC_MODE_BUFFERED) {
		pRegs->ADCR &= ~LM3Sxx_ADC_ADCR_BURST;
	}*/

	return BT_ERR_NONE;
}

static BT_s32 adc_Read(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest) {
	volatile LM3Sxx_ADC_REGS *pRegs = hAdc->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	BT_s32 slRead = 0;

	switch(hAdc->eMode) {
	case BT_ADC_MODE_POLLED:
	{
		while(ulSize) {
			BT_u32 i, j;
			BT_u32 ulSum = 0;
			BT_u32 ulResult;

			for (i = 0; i < hAdc->ulSWAverageCount; i++) {
				if (ulChannel < 8) {
					pRegs->ADCPSSI = LM3Sxx_ADC_ADCPSSI_SS0_START;
					while((pRegs->ADCRIS & LM3Sxx_ADC_ADCRIS_SS0)) {
						BT_ThreadYield();
					}
					for (j = 0; j < ulChannel; j++)
						ulResult = pRegs->Sequencer[0].SSFIFO;
				}
				else if (ulChannel < 12) {
					pRegs->ADCPSSI = LM3Sxx_ADC_ADCPSSI_SS1_START;
					while((pRegs->ADCRIS & LM3Sxx_ADC_ADCRIS_SS1)) {
						BT_ThreadYield();
					}
					for (j = 8; j < ulChannel; j++)
						ulResult = pRegs->Sequencer[1].SSFIFO;
				}
				else if (ulChannel < 16) {
					pRegs->ADCPSSI = LM3Sxx_ADC_ADCPSSI_SS2_START;
					while((pRegs->ADCRIS & LM3Sxx_ADC_ADCRIS_SS2)) {
						BT_ThreadYield();
					}
					for (j = 12; j < ulChannel; j++)
						ulResult = pRegs->Sequencer[2].SSFIFO;
				}
				else {
					pRegs->ADCPSSI = LM3Sxx_ADC_ADCPSSI_SS3_START;
					while((pRegs->ADCRIS & LM3Sxx_ADC_ADCRIS_SS3)) {
						BT_ThreadYield();
					}
					for (j = 16; j < ulChannel; j++)
						ulResult = pRegs->Sequencer[3].SSFIFO;
				}
				ulSum += ulResult;
			}

			*pucDest++ = ulSum / hAdc->ulSWAverageCount;
			ulSize--;
			slRead++;
		}

		break;
	}

	case BT_ADC_MODE_BUFFERED:
	{
		// Get bytes from RX buffer very quickly.
		slRead = BT_FifoRead(hAdc->hFifo[ulChannel], ulSize, pucDest, 0);
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;
	}
	return slRead;
}

/**
 *	This actually allows the adc'S to be clocked!
 **/
static void enableAdcPeripheralClock(BT_HANDLE hAdc) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hAdc->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[0] |= LM3Sxx_RCC_RCGC_ADC0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[0] |= LM3Sxx_RCC_RCGC_ADC1EN;
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
		LM3Sxx_RCC->RCGC[0] &= ~LM3Sxx_RCC_RCGC_ADC0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[0] &= ~LM3Sxx_RCC_RCGC_ADC1EN;
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
		if(LM3Sxx_RCC->RCGC[0] & LM3Sxx_RCC_RCGC_ADC0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[0] & LM3Sxx_RCC_RCGC_ADC1EN) {
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

const BT_IF_DEVICE BT_LM3Sxx_ADC_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_ADC,											///< Allow configuration through the ADC api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oAdcDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_ADC_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = adc_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE adc_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hAdc = NULL;
	BT_u32 i;

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

	hAdc->pRegs = (LM3Sxx_ADC_REGS *) pResource->ulStart;

	AdcSetPowerState(hAdc, BT_POWER_STATE_AWAKE);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_SetInterruptPriority(pResource->ulStart, 1);

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, adc_irq_handler, hAdc);
	if(Error) {
		goto err_free_out;
	}*/

	for (i = pResource->ulStart; i < pResource->ulEnd; i++)
		Error = BT_EnableInterrupt(i);

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
	.name 		= "LM3Sxx,adc",
	.pfnProbe	= adc_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_ADC_0
static const BT_RESOURCE oLM3Sxx_adc0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_ADC0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_ADC0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 30,
		.ulEnd				= 33,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_adc0_device = {
	.name 					= "LM3Sxx,adc",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_adc0_resources),
	.pResources 			= oLM3Sxx_adc0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_adc0_inode = {
	.szpName = "adc0",
	.pDevice = &oLM3Sxx_adc0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_ADC_1
static const BT_RESOURCE oLM3Sxx_adc1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_ADC1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_ADC1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 64,
		.ulEnd				= 67,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_adc1_device = {
	.name 					= "LM3Sxx,adc",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_adc1_resources),
	.pResources 			= oLM3Sxx_adc1_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_adc1_inode = {
	.szpName = "adc1",
	.pDevice = &oLM3Sxx_adc1_device,
};
#endif
