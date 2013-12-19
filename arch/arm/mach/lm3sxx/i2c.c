/**
 *	LM3Sxx Hal for BitThunder
 *	I2C Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional I2C device driver
 *	for BitThunder.
 *
 *	This driver should be easily ported to I2C peripherals on other processors with little effort.
 *
 *	@author		Robert Steinbauer <rsteinbauer@riegl.com>
 *	@copyright	(c)2012 Robert Steinbauer
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>	 				// Include all those wonderful BitThunder APIs.
#include "i2c.h"						// Includes a full hardware definition for the integrated I2Cs.
#include "rcc.h"						// Used for getting access to rcc regs, to determine the real Clock Freq.
#include "ioconfig.h"					// Used for getting access to IOCON regs.
#include <collections/bt_fifo.h>

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("LM3Sxx-I2C")
BT_DEF_MODULE_DESCRIPTION				("Simple I2C device for the LM3Sxx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a I2C driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	BT_I2C_BUS					i2c_master;
	LM3Sxx_I2C_REGS	   *pRegs;
};

static const BT_u32 g_I2C_PERIPHERAL[3] = {7,19,26};

static const BT_IF_HANDLE oHandleInterface;	// Prototype for the I2COpen function.
static void disablei2cPeripheralClock(BT_HANDLE hI2C);


BT_ERROR BT_NVIC_IRQ_24(void) {
	return 0;
}

BT_ERROR BT_NVIC_IRQ_53(void) {
	return 0;
}


static void ResetI2C(BT_HANDLE hI2C)
{
	volatile LM3Sxx_I2C_REGS *pRegs = hI2C->pRegs;

	pRegs->LM3Sxx_I2C_MCR = 0x00000000;

}

/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR i2cCleanup(BT_HANDLE hI2C) {
	ResetI2C(hI2C);

	// Disable peripheral clock.
	disablei2cPeripheralClock(hI2C);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->i2c_master.pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	return BT_ERR_NONE;
}

static BT_ERROR i2c_set_clock_rate(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockrate) {
	volatile LM3Sxx_I2C_REGS *pRegs = hI2C->pRegs;

	BT_u32 ulInputClk;
	BT_u32 ulClock;

	/*
	 *	We must determine the input clock frequency to the I2C peripheral.
	 */

	ulInputClk = BT_LM3Sxx_GetSystemFrequency();

	switch (eClockrate) {
	case BT_I2C_CLOCKRATE_100kHz: {
		ulClock = 100000;
		break;
	}
	case BT_I2C_CLOCKRATE_400kHz: {
		ulClock = 400000;
		break;
	}
	case BT_I2C_CLOCKRATE_1000kHz: {
		ulClock = 1000000;
		break;
	}
	case BT_I2C_CLOCKRATE_3400kHz: {
		ulClock = 3400000;
		break;
	}
	default: {
		ulClock = 100000;
		break;
	}
	}
	pRegs->LM3Sxx_I2C_MTPR = (ulInputClk / (ulClock*20)) - 1;

	return BT_ERR_NONE;
}

/**
 *	This actually allows the I2CS to be clocked!
 **/
static void enablei2cPeripheralClock(BT_HANDLE hI2C) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->i2c_master.pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_I2C0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_I2C1EN;
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
static void disablei2cPeripheralClock(BT_HANDLE hI2C) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->i2c_master.pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_I2C0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_I2C1EN;
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
static BT_BOOL isi2cPeripheralClockEnabled(BT_HANDLE hI2C) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->i2c_master.pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_I2C0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_I2C1EN) {
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
 *	This implements the I2C power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR i2cSetPowerState(BT_HANDLE hI2C, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disablei2cPeripheralClock(hI2C);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enablei2cPeripheralClock(hI2C);
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
 *	This implements the I2C power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR i2cGetPowerState(BT_HANDLE hI2C, BT_POWER_STATE *pePowerState) {
	if(isi2cPeripheralClockEnabled(hI2C)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

static BT_ERROR i2cDisable(BT_HANDLE hI2C) {
	hI2C->pRegs->LM3Sxx_I2C_MCR = 0x00000000;

	return BT_ERR_NONE;
}

static BT_ERROR i2cEnable(BT_HANDLE hI2C) {
	hI2C->pRegs->LM3Sxx_I2C_MCR = LM3Sxx_I2C_MCR_MASTER_MODE;

	return BT_ERR_NONE;
}


static BT_ERROR i2cRead(BT_HANDLE hI2C, BT_u16 usDevice, BT_u8 *pucDest, BT_u32 ulLength) {
	volatile LM3Sxx_I2C_REGS *pRegs = hI2C->pRegs;
	BT_ERROR Error = BT_ERR_NONE;

	pRegs->LM3Sxx_I2C_MSA = (usDevice<<1) | LM3Sxx_I2C_MASTER_SA_RS;

    // Send out the number of bytes requested.
	BT_u32 i;
	for (i=0; i<ulLength; i++) {
		pRegs->LM3Sxx_I2C_MICR = LM3Sxx_I2C_MICR_IC;

		// Initiate bus cycle
		BT_u32 ulCondition = LM3Sxx_I2C_MCS_RUN;
        if (i == 0) ulCondition |= LM3Sxx_I2C_MCS_START;
        if (i == ulLength-1) ulCondition |= LM3Sxx_I2C_MCS_STOP;
        else ulCondition |= LM3Sxx_I2C_MCS_DATACK;
        pRegs->LM3Sxx_I2C_MCS = ulCondition;

        // Wait for transmission to complete.
        while (!(pRegs->LM3Sxx_I2C_MRIS & LM3Sxx_I2C_MRIS_RIS));
    	if (pRegs->LM3Sxx_I2C_MCS & LM3Sxx_I2C_MCS_ADRACK) {
    		Error = -1;		// no address acknowledge
    		break;
    	}

        // Receive a byte from the I2C.
        *pucDest++ = pRegs->LM3Sxx_I2C_MDR;
    }
	while (pRegs->LM3Sxx_I2C_MCS & LM3Sxx_I2C_MCS_BUSBSY);

	return Error;
}

/**
 *	Implementing the CHAR dev write API.
 *
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR i2cWrite(BT_HANDLE hI2C, BT_u16 usDevice, BT_u8 *pucSource, BT_u32 ulLength) {
	volatile LM3Sxx_I2C_REGS *pRegs = hI2C->pRegs;
	BT_ERROR Error = BT_ERR_NONE;

	pRegs->LM3Sxx_I2C_MSA = usDevice<<1;

	BT_u32 i;

	for (i=0; i<ulLength; i++) {
		pRegs->LM3Sxx_I2C_MICR = LM3Sxx_I2C_MICR_IC;

		// Send out the next byte.
        pRegs->LM3Sxx_I2C_MDR = *pucSource++;

        // Initiate bus cycle
        BT_u32 ulCondition = LM3Sxx_I2C_MCS_RUN;
        if (i == 0) ulCondition |= LM3Sxx_I2C_MCS_START;
        if (i == ulLength-1) ulCondition |= LM3Sxx_I2C_MCS_STOP;
        pRegs->LM3Sxx_I2C_MCS = ulCondition;

        // Wait for transmission to complete.
        while (!(pRegs->LM3Sxx_I2C_MRIS & LM3Sxx_I2C_MRIS_RIS));

    	if (pRegs->LM3Sxx_I2C_MCS & (LM3Sxx_I2C_MCS_ADRACK | LM3Sxx_I2C_MCS_DATACK)) {
    		Error = -1;		// no address acknowledge
    		break;
    	}
	}

	return Error;
}

static BT_u32 i2c_master_transfer(BT_HANDLE hI2C, BT_I2C_MESSAGE *msgs, BT_u32 num, BT_ERROR *pError) {
	if (pError) *pError = BT_ERR_NONE;

	BT_u32 count;
	for(count = 0; count < num; count++, msgs++) {
		if(msgs->flags & BT_I2C_M_RD) {
			i2cRead(hI2C, msgs->addr, msgs->buf, msgs->len);
		}
		else {
			i2cWrite(hI2C, msgs->addr, msgs->buf, msgs->len);
		}
	}

	return num;
}

static const BT_IF_POWER oPowerInterface = {
	.pfnSetPowerState	= i2cSetPowerState,											///< Pointers to the power state API implementations.
	.pfnGetPowerState	= i2cGetPowerState,											///< This gets the current power state.
};

static const BT_DEV_IF_I2C oI2CInterface = {
	.ulFunctionality	= BT_I2C_FUNC_I2C,
	.pfnMasterTransfer	= i2c_master_transfer,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pPowerIF			= &oPowerInterface,											///< Device does not support powerstate functionality.
	.eConfigType		= BT_DEV_IF_T_I2C,											///< Allow configuration through the I2C api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oI2CInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup	= i2cCleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE i2c_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hI2C = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hI2C = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hI2C) {
		goto err_out;
	}

	hI2C->pRegs = (LM3Sxx_I2C_REGS *) pResource->ulStart;


	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hI2C->i2c_master.pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_INTEGER, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	i2cSetPowerState(hI2C, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults!
	ResetI2C(hI2C);

	i2cEnable(hI2C);

	i2c_set_clock_rate(hI2C, pResource->ulStart);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, I2C_irq_handler, hI2C);
	if(Error) {
		goto err_free_out;
	}*/


	//@@Error = BT_EnableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_BUSID, 0);
	if(!pResource) {
		goto err_free_out;
	}

	BT_u32 ulBusID = pResource->ulStart;

	hI2C->i2c_master.ulBusID	= ulBusID;
	hI2C->i2c_master.hBus 		= hI2C;		// Save pointer to bus's private data.
	hI2C->i2c_master.pDevice 	= pDevice;


	BT_I2C_RegisterBus(&hI2C->i2c_master);

	if(pError) {
		*pError = Error;
	}

	return hI2C;

err_free_out:
	BT_DestroyHandle(hI2C);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF I2C_driver = {
	.name 		= "LM3Sxx,i2c",
	.pfnProbe	= i2c_probe,
};


