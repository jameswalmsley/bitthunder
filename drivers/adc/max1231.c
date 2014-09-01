/**
 *	MAX1231 and derivatives driver.
 *
 *	@author Robert Steinbauer <rsteinbauer@riegl.co.at>
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME				("Maxim MAX1231 ADC")
BT_DEF_MODULE_DESCRIPTION		("Microchip SPI ADC")
BT_DEF_MODULE_AUTHOR			("Robert Steinbauer")
BT_DEF_MODULE_EMAIL				("rsteinbauer@riegl.co.at")


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;
	BT_SPI_DEVICE 		   *pSpi;
	BT_ADC_OPERATING_MODE	eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_ADC_INFO		 		oInfo;
	BT_u32					ulGpio_CS;
	BT_u32					ulGpio_EOC;
};

static BT_ERROR adc_cleanup(BT_HANDLE hAdc) {

	hAdc->oInfo.ulReferenceCount--;
	
	return BT_ERR_NONE;
}

static BT_ERROR adc_setconfig(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 i;
	BT_u8 ucConfig[2];

	// Setup Register with unipolar mode register
	ucConfig[0] = 0x60;
	if (pConfig->bUseIntReference)
		ucConfig[0] |= 0x08;
	else
		ucConfig[0] |= 0x04;

	if (pConfig->eInputMode == BT_ADC_INPUT_MODE_DIFFERENTIAL)
		ucConfig[1] = 0xFF;
	else ucConfig[1] = 0x00;

	ucConfig[2] = 0x20;
	if (pConfig->ulHWAverageCount > 4) {
		ucConfig[2] |= 0x10;
		BT_u8 ucAvg = pConfig->ulHWAverageCount >> 3;
		if (ucAvg > 3) ucAvg = 3;
		ucConfig[2] |= ucAvg << 2;
	}

	BT_GpioSet(hAdc->ulGpio_CS, BT_FALSE);
	BT_SpiWrite(hAdc->pSpi, (void*)&ucConfig[0], 3);
	BT_GpioSet(hAdc->ulGpio_CS, BT_TRUE);

	switch(pConfig->eMode) {
	case BT_ADC_MODE_POLLED: {
		hAdc->eMode = BT_ADC_MODE_POLLED;
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

	pConfig->ulBufferSize		= 1;
	pConfig->eMode				= BT_ADC_MODE_POLLED;
	pConfig->ulResolution		= 12;

	return BT_ERR_NONE;
}


static BT_s32 adc_Read(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pDest) {
	BT_ERROR Error = BT_ERR_NONE;

	BT_u8 ucConversion;
	BT_u8 ucBuffer[4];

	switch(hAdc->eMode) {
	case BT_DAC_MODE_POLLED:
	{
		ucConversion = 0x87;
		if (ulChannel < 16)
			ucConversion |= (ulChannel << 3);

		while(ulSize--) {

			BT_GpioSet(hAdc->ulGpio_CS, BT_FALSE);
			BT_SpiWrite(hAdc->pSpi, (void*)&ucConversion, 1);
			BT_GpioSet(hAdc->ulGpio_CS, BT_TRUE);

			while (BT_GpioGet(hAdc->ulGpio_EOC, &Error)) {
				BT_ThreadYield();
			}

			BT_GpioSet(hAdc->ulGpio_CS, BT_FALSE);
			BT_SpiRead(hAdc->pSpi, (void*)&ucBuffer, 4);
			BT_GpioSet(hAdc->ulGpio_CS, BT_TRUE);

			BT_u32 ulResult = ((BT_u32)ucBuffer[0]<<8) + ucBuffer[1];
			if (ulChannel < 16)
				ulResult = ((BT_u32)ucBuffer[2]<<8) + ucBuffer[3];
			*pDest++ = ulResult;
		}
		break;
	}
	default: {
		// ERR, invalid handle configuration.
		break;
	}
	}

	return BT_ERR_NONE;
}


static const BT_DEV_IF_ADC adc_ops = {
	.pfnSetConfig 		= adc_setconfig,
	.pfnGetConfig		= adc_getconfig,
	.pfnRead			= adc_Read,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_ADC,
	.unConfigIfs = {
		.pADCIF = &adc_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.pfnCleanup = adc_cleanup,												///< Handle's cleanup routine.
};


static BT_HANDLE adc_probe(BT_SPI_DEVICE *spi, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	spi->bits_per_word = 8;
	spi->chip_select = 0;
	spi->mode = spi->pMaster->mode_bits;
	spi->max_speed_hz = 8000000;

	//Error = BT_SpiSetup(spi);

	BT_HANDLE hAdc = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hAdc) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hAdc->pSpi    = spi;	// Provide access to parameters on cleanup.

	const BT_RESOURCE *pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_IO, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	hAdc->ulGpio_CS = pResource->ulStart;
	hAdc->ulGpio_EOC = pResource->ulEnd;

	BT_GpioSetDirection(hAdc->ulGpio_CS, BT_GPIO_DIR_OUTPUT);
	BT_GpioSetDirection(hAdc->ulGpio_EOC, BT_GPIO_DIR_INPUT);
	BT_GpioSet(hAdc->ulGpio_CS, BT_TRUE);

	hAdc->oInfo.pDevice = pDevice;

	BT_AdcRegisterDevice(hAdc, &hAdc->oInfo);

	return hAdc;

err_free_out:
	BT_DestroyHandle(hAdc);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 	= "maxim,1231",
	.eType 	= BT_DRIVER_SPI,
	.pfnSPIProbe = adc_probe,
};
