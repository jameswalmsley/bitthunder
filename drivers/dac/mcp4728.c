/**
 *	MC4728 and derivatives driver.
 *
 *	@author Robert Steinbauer <rsteinbauer@riegl.co.at>
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME				("Microchip MCP4728 quad DAC")
BT_DEF_MODULE_DESCRIPTION		("Microchip I2C DAC")
BT_DEF_MODULE_AUTHOR			("Robert Steinbauer")
BT_DEF_MODULE_EMAIL				("rsteinbauer@riegl.co.at")


#define MCP4728_WRITE_GAIN		0xC0
#define MCP4728_WRITE_VREF		0x80
#define	MCP4728_FAST_WRITE		0x00
#define	MCP4728_SEQ_WRITE		0x40


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;
	BT_DAC_OPERATING_MODE	eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_DAC_INFO		 		oInfo;
	BT_I2C_CLIENT	 		oClient;
	BT_I2C_MESSAGE	 		oMessages;
	BT_BOOL					bUseInternalReference;
	BT_u32					ulGain;
};

static BT_ERROR dac_cleanup(BT_HANDLE hDac) {

	hDac->oInfo.ulReferenceCount--;

	return BT_ERR_NONE;
}

static BT_ERROR dac_setconfig(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 i;
	BT_u8 ucBuffer;

	ucBuffer = MCP4728_WRITE_GAIN;
	for (i = 0; i < 4; i++)
		if ((pConfig->ulGain % 2) == 0)
			ucBuffer |= 0x01 << i;

	hDac->oMessages.addr 	= hDac->oClient.addr;
	hDac->oMessages.len		= 1;
	hDac->oMessages.buf		= &ucBuffer;
	hDac->oMessages.flags 	= 0;

	BT_I2C_Transfer(hDac->oClient.pBus, &hDac->oMessages, 1, &Error);

	hDac->bUseInternalReference = pConfig->bInternalReference;

	ucBuffer = MCP4728_WRITE_VREF;
	for (i = 0; i < 4; i++)
		if (pConfig->bInternalReference)
			ucBuffer |= 0x01 << i;

	BT_I2C_Transfer(hDac->oClient.pBus, &hDac->oMessages, 1, &Error);

	switch(pConfig->eMode) {
	case BT_DAC_MODE_POLLED: {
		hDac->eMode = BT_DAC_MODE_POLLED;
		break;
	}
	default:
		// Unsupported operating mode!
		break;
	}

	return BT_ERR_NONE;
}

static BT_ERROR dac_getconfig(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;

	pConfig->ulBufferSize		= 1;
	pConfig->eMode				= BT_DAC_MODE_POLLED;
	pConfig->ulUpdateInterval 	= 0;
	pConfig->ulResolution		= 12;

	return BT_ERR_NONE;
}


static BT_ERROR dac_Write(BT_HANDLE hDac, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pSrc) {
	BT_ERROR Error = BT_ERR_NONE;

	static BT_u8 ucBuffer[3];


	switch(hDac->eMode) {
	case BT_DAC_MODE_POLLED:
	{
		while(ulSize--) {
			BT_u16 uValue = (*pSrc & 0x0FFF);

			ucBuffer[0] = MCP4728_SEQ_WRITE | ulChannel << 1;
			ucBuffer[1] = ((uValue >> 8)) & 0x0F;
			ucBuffer[2] = (uValue & 0xFF);

			if (hDac->bUseInternalReference)
				ucBuffer[1] |= 0x80;
			if (hDac->ulGain % 2)
				ucBuffer[1] |= 0x10;

			hDac->oMessages.addr 	= hDac->oClient.addr;
			hDac->oMessages.len		= 3;
			hDac->oMessages.buf		= ucBuffer;
			hDac->oMessages.flags 	= 0;

			BT_I2C_Transfer(hDac->oClient.pBus, &hDac->oMessages, 1, &Error);

			break;
		}
		default:
			// ERR, invalid handle configuration.
			break;
		}
	}
	return BT_ERR_NONE;
}


static const BT_DEV_IF_DAC dac_ops = {
	.pfnSetConfig 		= dac_setconfig,
	.pfnGetConfig		= dac_getconfig,
	.pfnWrite			= dac_Write,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_DAC,
	.unConfigIfs = {
		.pDACIF = &dac_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.pfnCleanup = dac_cleanup,												///< Handle's cleanup routine.
};


static BT_HANDLE dac_probe(BT_HANDLE hBus, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_I2C_BUS *pBus = BT_I2C_GetBusObject(hBus);

	BT_HANDLE hDac = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hDac) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hDac->oClient.pBus = pBus;

	const BT_RESOURCE *pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_ADDR, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	hDac->oClient.addr = pResource->ulStart;

	BT_kPrint("Probing DAC MCP4728 on I2C-bus %d and I2C-address 0x%02x", hDac->oClient.pBus->ulBusID, hDac->oClient.addr);


	hDac->oInfo.pDevice = pDevice;

	BT_DacRegisterDevice(hDac, &hDac->oInfo);

	return hDac;

err_free_out:
	BT_DestroyHandle(hDac);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 	= "microchip,mcp4728",
	.eType 	= BT_DRIVER_I2C,
	.pfnI2CProbe = dac_probe,
};
