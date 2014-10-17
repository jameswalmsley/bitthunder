/**
 *	MAX1363 and derivatives driver.
 *
 *	@author Robert Steinbauer <rsteinbauer@riegl.co.at>
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME				("Maxim Max1363 quad ADC")
BT_DEF_MODULE_DESCRIPTION		("Maxim I2C ADC")
BT_DEF_MODULE_AUTHOR			("Robert Steinbauer")
BT_DEF_MODULE_EMAIL				("rsteinbauer@riegl.co.at")




struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;
	BT_ADC_OPERATING_MODE	eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_ADC_INFO		 		oInfo;
	BT_I2C_CLIENT	 		oClient;
	BT_I2C_MESSAGE	 		oMessages;
	BT_ADC_INPUT_MODE		eInputMode;
};

static BT_ERROR adc_cleanup(BT_HANDLE hAdc) {

	hAdc->oInfo.ulReferenceCount--;

	return BT_ERR_NONE;
}

static BT_ERROR adc_setconfig(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 i;
	BT_u8 ucSetup;

	// Setup Register with unipolar mode register
	ucSetup = 0x92;
	if (pConfig->bUseIntReference)
		ucSetup |= 0x40;
	else
		ucSetup |= 0x20;


	hAdc->oMessages.addr 	= hAdc->oClient.addr;
	hAdc->oMessages.len		= 1;
	hAdc->oMessages.buf		= &ucSetup;
	hAdc->oMessages.flags 	= 0;

	BT_I2C_Transfer(hAdc->oClient.pBus, &hAdc->oMessages, 1, &Error);

	hAdc->eInputMode       = pConfig->eInputMode;

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
	BT_u8 ucBuffer[2];

	switch(hAdc->eMode) {
	case BT_ADC_MODE_POLLED:
	{
		while(ulSize--) {

			BT_u8 ucConfig = 0x60 | (ulChannel << 1);

			if (hAdc->eInputMode == BT_ADC_INPUT_MODE_SINGLE_END)
				ucConfig |= 0x01;

			hAdc->oMessages.addr 	= hAdc->oClient.addr;
			hAdc->oMessages.len		= 1;
			hAdc->oMessages.buf		= &ucConfig;
			hAdc->oMessages.flags 	= 0;

			BT_I2C_Transfer(hAdc->oClient.pBus, &hAdc->oMessages, 1, &Error);

			hAdc->oMessages.addr 	= hAdc->oClient.addr;
			hAdc->oMessages.len		= 2;
			hAdc->oMessages.buf		= ucBuffer;
			hAdc->oMessages.flags 	= BT_I2C_M_RD;

			BT_I2C_Transfer(hAdc->oClient.pBus, &hAdc->oMessages, 1, &Error);

			*pDest++ = (((BT_u32)ucBuffer[0]<<8) + ucBuffer[1]) & 0xFFF;
		}
		default:
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


static BT_HANDLE adc_probe(BT_HANDLE hBus, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_I2C_BUS *pBus = BT_I2C_GetBusObject(hBus);

	BT_HANDLE hAdc = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hAdc) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hAdc->oClient.pBus = pBus;

	const BT_RESOURCE *pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_ADDR, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	hAdc->oClient.addr = pResource->ulStart;

	BT_kPrint("Probing ADC MAX1363 on I2C-bus %d and I2C-address 0x%02x", hAdc->oClient.pBus->ulBusID, hAdc->oClient.addr);


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
	.name 	= "maxim,1363",
	.eType 	= BT_DRIVER_I2C,
	.pfnI2CProbe = adc_probe,
};
