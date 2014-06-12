/**
 *	Provides the LM3Sxx mac for BitThunder.
 **/

#include <bitthunder.h>
#include "mac.h"
#include "rcc.h"
#include <collections/bt_fifo.h>


BT_DEF_MODULE_NAME			("LM3Sxx-mac")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx mac kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a mac driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;				///< All handles must include a handle header.
	BT_NET_IF				   *pIF;
	LM3Sxx_MAC_REGS   	   	   *pRegs;
	const BT_INTEGRATED_DEVICE *pDevice;
	BT_NET_IF_EVENTRECEIVER	    pfnEvent;
	BT_HANDLE 					hMII;
};


struct _MII_HANDLE {
	BT_HANDLE_HEADER		h;
	struct bt_mii_bus		mii;
};

static const BT_IF_HANDLE oMIIHandleInterface;


static BT_HANDLE g_mac_HANDLES[1] = {
	NULL,
};

static void disablemacPeripheralClock(BT_HANDLE hMAC);

BT_ERROR BT_NVIC_IRQ_58(void) {
	BT_HANDLE hMAC = g_mac_HANDLES[0];
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;
    //BT_u32 ulWake;

    // Read and Clear the interrupt.
    BT_u32 ulStatus = pRegs->MACRIS;
    pRegs->MACIACK = ulStatus;

    // A RTOS is being used.  Signal the ethernet interrupt task.
    if (ulStatus) {
    	hMAC->pfnEvent(hMAC->pIF, BT_NET_IF_RX_READY, BT_TRUE);
    }
    else {
    	while (1);
    }


    // Disable the ethernet interrupts.  Since the interrupts have not been
    // handled, they are not asserted.  Once they are handled by the ethernet
    // interrupt task, it will re-enable the interrupts.

    pRegs->MACIM &= ~(LM3Sxx_MAC_MACIM_RXINT | LM3Sxx_MAC_MACIM_TXEMP);

    return BT_ERR_NONE;
}


static struct _MII_HANDLE* mii_init(BT_HANDLE hMac, BT_ERROR *pError) {
	struct _MII_HANDLE *pMII = (struct _MII_HANDLE *) BT_CreateHandle(&oMIIHandleInterface, sizeof(struct _MII_HANDLE), pError);
	if(!pMII) {
		if(pError) {
			*pError = BT_ERR_NO_MEMORY;
		}
		return NULL;
	}

	hMac->hMII = (BT_HANDLE) pMII;

	pMII->mii.hMAC = hMac;
	pMII->mii.name = "LM3Sxx mac-mii bus";

	return pMII;
}

static BT_u16 mii_read(BT_HANDLE hMII, BT_u32 phy_id, BT_u32 regnum, BT_ERROR *pError) {

	/*struct _MII_HANDLE *pMII = (struct _MII_HANDLE *) hMII;
	while(!(pMII->mii.hMAC->pRegs->net_status & NET_STATUS_MGMT_IDLE)) {
		BT_ThreadYield();
	}

	BT_u32 reg = (phy_id << 23) & PHY_MAINT_PHY_ADDR;
	reg |= (regnum << 18) & PHY_MAINT_REG_ADDR;
	reg |= PHY_MAINT_R_MASK;
	reg |= 0x40020000;

	pMII->mii.hMAC->pRegs->phy_maint = reg;

	while(!(pMII->mii.hMAC->pRegs->net_status & NET_STATUS_MGMT_IDLE)) {
		BT_ThreadYield();
	}

	return pMII->mii.hMAC->pRegs->phy_maint & PHY_MAINT_DATA;*/
	return 0;
}

static BT_ERROR mii_write(BT_HANDLE hMII, BT_u32 phy_id, BT_u32 regnum, BT_u16 val) {

	/*struct _MII_HANDLE *pMII = (struct _MII_HANDLE *) hMII;
	while(!(pMII->mii.hMAC->pRegs->net_status & NET_STATUS_MGMT_IDLE)) {
		BT_ThreadYield();
	}

	BT_u32 reg = (phy_id << 23) & PHY_MAINT_PHY_ADDR;
	reg |= (regnum << 18) & PHY_MAINT_REG_ADDR;
	reg |= PHY_MAINT_W_MASK;
	reg |= 0x40020000;
	reg |= val;

	pMII->mii.hMAC->pRegs->phy_maint = reg;

	while(!(pMII->mii.hMAC->pRegs->net_status & NET_STATUS_MGMT_IDLE)) {
		BT_ThreadYield();
	}*/

	return BT_ERR_NONE;
}

static BT_ERROR mii_reset(BT_HANDLE hMII) {
	return BT_ERR_NONE;
}

static BT_ERROR mii_cleanup(BT_HANDLE hMII) {
	return BT_ERR_NONE;
}


static BT_ERROR mac_send_event(BT_HANDLE hMAC, BT_u32 ulEvent) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	switch (ulEvent) {
	case BT_MAC_RECEIVED: {
		pRegs->MACIM |= LM3Sxx_MAC_MACIM_RXINT | LM3Sxx_MAC_MACIM_TXEMP;		//@@ST
		break;
	}
	case BT_MAC_TRANSMITTED: {
		pRegs->MACIM |= LM3Sxx_MAC_MACIM_TXEMP;
		break;
	}
	default :
		break;
	}

	return BT_ERR_NONE;
}


static BT_ERROR mac_cleanup(BT_HANDLE hMAC) {

	// Disable peripheral clock.
	disablemacPeripheralClock(hMAC);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMAC->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hMAC->pDevice, BT_RESOURCE_ENUM, 0);

	g_mac_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

BT_ERROR mac_eventsubscribe(BT_HANDLE hIF, BT_NET_IF_EVENTRECEIVER pfnReceiver, BT_NET_IF *pIF) {

	hIF->pfnEvent = pfnReceiver;
	hIF->pIF	  = pIF;

	return BT_ERR_NONE;
}

static BT_u32 mac_dataready(BT_HANDLE hMAC, BT_ERROR *pError) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	if (pError) *pError = BT_ERR_NONE;

	if (pRegs->MACNP & LM3Sxx_MAC_MACNP_M) {
		return pRegs->MACDATA;
	}

	return 0;
}

static BT_ERROR mac_read(BT_HANDLE hMAC, BT_u32 ulSize, BT_u32 ulPos, void *pucDest) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	BT_u32 *pDest = (BT_u32*)pucDest + ulPos;

	BT_u32 i;
	for (i = 0; i < ulSize; i+=4) {
		*pDest++ = pRegs->MACDATA;
	}

	return BT_ERR_NONE;
}

static BT_ERROR mac_drop(BT_HANDLE hMAC, BT_u32 ulSize) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	BT_u32 i;
	for (i = 0; i < ulSize; i+=4) {
		pRegs->MACDATA;
	}

	return BT_ERR_NONE;

}

static BT_BOOL mac_txfifoready(BT_HANDLE hMAC, BT_ERROR *pError) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	if (pError) *pError = BT_ERR_NONE;

	if (!(pRegs->MACTR & LM3Sxx_MAC_MACTR_NEWTX)) return BT_TRUE;

	return BT_FALSE;
}


static BT_u32  iGather = 0;
static BT_u32  ulGather = 0;

static BT_ERROR mac_write(BT_HANDLE hMAC, BT_u32 ulSize, void *pucSrc) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	BT_u32 *pSrc;
	BT_u8  *pucBuf = (BT_u8 *)pucSrc;
	BT_u32  iBuf = 0;
	BT_u8  *pucGather = (unsigned char *)&ulGather;

	while((iBuf < ulSize) && (iGather != 0)) {
		/* Copy a byte from the pbuf into the gather buffer. */
		pucGather[iGather] = pucBuf[iBuf++];
		/* Increment the gather buffer index modulo 4. */
		iGather = ((iGather + 1) % 4);
	}

	/**
	* If the gather index is 0 and the pbuf index is non-zero,
	* we have a gather buffer to write into the Tx FIFO.
	*
	*/
	if((iGather == 0) && (iBuf != 0)) {
		pRegs->MACDATA = ulGather;
		ulGather = 0;
	}

	/* Initialze a long pointer into the pbuf for 32-bit access. */
	pSrc = (unsigned long *)&pucBuf[iBuf];

	BT_u32 ulLen = ulSize/4;

	while (ulLen--) {
		pRegs->MACDATA = *pSrc++;
	}

	iBuf += (ulSize/4)*4;

	while(iBuf < ulSize) {
		/* Copy a byte from the pbuf into the gather buffer. */
		pucGather[iGather] = pucBuf[iBuf++];
		/* Increment the gather buffer index modulo 4. */
		iGather = ((iGather + 1) % 4);
	}

	return BT_ERR_NONE;
}

static BT_ERROR mac_sendframe(BT_HANDLE hMAC) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	pRegs->MACDATA = ulGather;

	pRegs->MACTR |= LM3Sxx_MAC_MACTR_NEWTX;

	iGather = 0;
	ulGather = 0;

	return BT_ERR_NONE;
}

/* get MAC hardware address */
static BT_ERROR mac_getaddr(BT_HANDLE hIF, BT_u8 *pucAddr, BT_u32 ulLength) {
	volatile LM3Sxx_MAC_REGS *pRegs = hIF->pRegs;
    BT_u32 ulTemp;
    BT_u8 *pucTemp = (BT_u8 *)&ulTemp;

    // Read the MAC address from the device.  The first four bytes of the
    // MAC address are read from the IA0 register.  The remaining two bytes
    // of the MAC addres
    ulTemp = pRegs->MACIA0;
    pucAddr[0] = pucTemp[0];
    pucAddr[1] = pucTemp[1];
    pucAddr[2] = pucTemp[2];
    pucAddr[3] = pucTemp[3];

    ulTemp = pRegs->MACIA1;
    pucAddr[4] = pucTemp[0];
    pucAddr[5] = pucTemp[1];

    return BT_ERR_NONE;
}

/* set MAC hardware address */
static BT_ERROR mac_setaddr(BT_HANDLE hIF, const BT_u8 *pucAddr, BT_u32 ulLength) {
	volatile LM3Sxx_MAC_REGS *pRegs = hIF->pRegs;
    BT_u32 ulTemp;
    BT_u8 *pucTemp = (BT_u8 *)&ulTemp;

    // Program the MAC Address into the device.  The first four bytes of the
    // MAC Address are placed into the IA0 register.  The remaining two bytes
    // of the MAC address are placed into the IA1 register.
    pucTemp[0] = pucAddr[0];
    pucTemp[1] = pucAddr[1];
    pucTemp[2] = pucAddr[2];
    pucTemp[3] = pucAddr[3];
    pRegs->MACIA0 = ulTemp;

    ulTemp = 0;
    pucTemp[0] = pucAddr[4];
    pucTemp[1] = pucAddr[5];
    pRegs->MACIA1 = ulTemp;

    return BT_ERR_NONE;
}

static BT_u32 mac_getmtusize(BT_HANDLE hIF, BT_ERROR *pError) {
	/* maximum transfer unit */
	if (pError) *pError = BT_ERR_NONE;

	return 1500;
}

static BT_ERROR mac_init(BT_HANDLE hMAC) {
	volatile LM3Sxx_MAC_REGS *pRegs = hMAC->pRegs;

	BT_u32 ulTemp;

	BT_ERROR Error = BT_ERR_NONE;

	struct _MII_HANDLE *pMII = mii_init(hMAC, &Error);
	if(!pMII) {
		return Error;
	}

	Error = BT_RegisterMiiBus((BT_HANDLE) pMII, &pMII->mii);
	if(Error) {
		return Error;
	}


	/* Do whatever else is needed to initialize interface. */
	/* Disable all mac Interrupts. */
	pRegs->MACIM &= ~(LM3Sxx_MAC_MACIM_PHYINT |
					  LM3Sxx_MAC_MACIM_MDINT |
					  LM3Sxx_MAC_MACIM_RXER |
					  LM3Sxx_MAC_MACIM_FOV |
					  LM3Sxx_MAC_MACIM_RXINT |
					  LM3Sxx_MAC_MACIM_TXEMP |
					  LM3Sxx_MAC_MACIM_TXER);

	ulTemp = pRegs->MACRIS;
	pRegs->MACIACK = ulTemp;

	BT_u8 ucMAC[6] = {0x00, 0x50, 0x45, 0x67, 0x89, 0xEF};
	mac_setaddr(hMAC, ucMAC, 6);

	/* Initialize the mac Controller. */
	BT_u32 ulInputClk = BT_LM3Sxx_GetMainFrequency();
    BT_u32 ulDiv = (ulInputClk / 2) / 2500000;
    pRegs->MACMDV = (ulDiv & 0xFF);

	/*
	* Configure the mac Controller for normal operation.
	* - Enable TX Duplex Mode
	* - Enable TX Padding
	* - Enable TX CRC Generation
	* - Enable RX Multicast Reception
	*/
	// Setup the Transmit Control Register.
    ulTemp  = pRegs->MACTCTL;
    ulTemp &= ~(LM3Sxx_MAC_MACTCTL_DUPLEX | LM3Sxx_MAC_MACTCTL_CRC | LM3Sxx_MAC_MACTCTL_PADEN);
    ulTemp |= (LM3Sxx_MAC_MACTCTL_DUPLEX | LM3Sxx_MAC_MACTCTL_CRC | LM3Sxx_MAC_MACTCTL_PADEN);
    pRegs->MACTCTL = ulTemp;

    // Setup the Receive Control Register.
    ulTemp  = pRegs->MACRCTL;
    ulTemp &= ~(LM3Sxx_MAC_MACRCTL_BADCRC | LM3Sxx_MAC_MACRCTL_PRMS | LM3Sxx_MAC_MACRCTL_AMUL);
    ulTemp |= LM3Sxx_MAC_MACRCTL_AMUL;
    pRegs->MACRCTL = ulTemp;

    // Setup the Time Stamp Configuration register.
    ulTemp = pRegs->MACTS;
    ulTemp &= ~(LM3Sxx_MAC_MACTS_TSEN);
    pRegs->MACTS = ulTemp;

	/* Enable the mac Controller transmitter and receiver. */
    // Reset the receive FIFO.
    pRegs->MACRCTL |= LM3Sxx_MAC_MACRCTL_RSTFIFO;

    // Enable the Ethernet receiver.
    pRegs->MACRCTL |= LM3Sxx_MAC_MACRCTL_RXEN;

    // Enable Ethernet transmitter.
    pRegs->MACTCTL |= LM3Sxx_MAC_MACTCTL_TXEN;

    // Reset the receive FIFO again, after the receiver has been enabled.
    pRegs->MACRCTL |= LM3Sxx_MAC_MACRCTL_RSTFIFO;

	/* Enable mac TX and RX Packet Interrupts. */
	pRegs->MACIM |= (LM3Sxx_MAC_MACIM_TXEMP | LM3Sxx_MAC_MACIM_RXINT);

	return BT_ERR_NONE;
}

/**
 *	This actually allows the mac'S to be clocked!
 **/
static void enablemacPeripheralClock(BT_HANDLE hMAC) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMAC->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[2] |= LM3Sxx_RCC_RCGC_MACEN;
		LM3Sxx_RCC->RCGC[2] |= LM3Sxx_RCC_RCGC_PHYEN;
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
static void disablemacPeripheralClock(BT_HANDLE hMAC) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMAC->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[2] &= ~LM3Sxx_RCC_RCGC_MACEN;
		LM3Sxx_RCC->RCGC[2] &= ~LM3Sxx_RCC_RCGC_PHYEN;
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
static BT_BOOL ismacPeripheralClockEnabled(BT_HANDLE hMAC) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMAC->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if ((LM3Sxx_RCC->RCGC[2] & LM3Sxx_RCC_RCGC_MACEN) &&
			(LM3Sxx_RCC->RCGC[2] & LM3Sxx_RCC_RCGC_PHYEN)) {
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
 *	This implements the mac power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR macSetPowerState(BT_HANDLE hMAC, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disablemacPeripheralClock(hMAC);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enablemacPeripheralClock(hMAC);
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
 *	This implements the mac power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR macGetPowerState(BT_HANDLE hMAC, BT_POWER_STATE *pePowerState) {
	if(ismacPeripheralClockEnabled(hMAC)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_MII mii_ops = {
	.pfnRead 	= mii_read,
	.pfnWrite 	= mii_write,
	.pfnReset 	= mii_reset,
};

static const BT_IF_DEVICE oMIIDeviceIF = {
	.eConfigType	= BT_DEV_IF_T_MII,
	.unConfigIfs 	= {
		.pMiiIF = &mii_ops,
	},
};

static const BT_IF_HANDLE oMIIHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oMIIDeviceIF,
	},
	.eType = BT_HANDLE_T_DEVICE,
	.pfnCleanup = mii_cleanup,
};


static const BT_DEV_IF_EMAC mac_ops = {
	.ulCapabilities		= BT_NET_IF_CAPABILITIES_ETHERNET | BT_NET_IF_CAPABILITIES_100MBPS,
	.pfnEventSubscribe	= mac_eventsubscribe,
	.pfnInitialise		= mac_init,
	.pfnGetMACAddr		= mac_getaddr,
	.pfnSetMACAddr		= mac_setaddr,
	.pfnGetMTU			= mac_getmtusize,
	.pfnDataReady		= mac_dataready,
	.pfnRead			= mac_read,
	.pfnWrite			= mac_write,
	.pfnDropFrame		= mac_drop,
	.pfnTxFifoReady		= mac_txfifoready,
	.pfnSendFrame		= mac_sendframe,
	.pfnSendEvent		= mac_send_event,
};

static const BT_IF_POWER oPowerInterface = {
	macSetPowerState,											///< Pointers to the power state API implementations.
	macGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LM3Sxx_mac_oDeviceInterface = {
	.pPowerIF		= &oPowerInterface,											///< Device does support powerstate functionality.
	.eConfigType	= BT_DEV_IF_T_EMAC,											///< Allow configuration through the mac api.
	.unConfigIfs 	= {
		.pEMacIF = &mac_ops,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_mac_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = mac_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE mac_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hMAC = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_mac_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hMAC = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hMAC) {
		goto err_out;
	}

	g_mac_HANDLES[pResource->ulStart] = hMAC;

	hMAC->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hMAC->pRegs = (LM3Sxx_MAC_REGS *) pResource->ulStart;

	macSetPowerState(hMAC, BT_POWER_STATE_AWAKE);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, mac_irq_handler, hMAC);
	if(Error) {
		goto err_free_out;
	}*/


	BT_SetInterruptPriority(pResource->ulStart, 3);
	Error = BT_EnableInterrupt(pResource->ulStart);

	Error = BT_RegisterNetworkInterface(hMAC);

	return hMAC;

err_free_out:
	BT_kFree(hMAC);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF mac_driver = {
	.name 		= "LM3Sxx,mac",
	.pfnProbe	= mac_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_MAC_0
static const BT_RESOURCE oLM3Sxx_mac0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_MAC0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_MAC0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 58,
		.ulEnd				= 58,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

BT_INTEGRATED_DEVICE_DEF oLM3Sxx_mac0_device = {
	.name 					= "LM3Sxx,mac",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_mac0_resources),
	.pResources 			= oLM3Sxx_mac0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_mac0_inode = {
	.szpName = "mac0",
	.pDevice = &oLM3Sxx_mac0_device,
};
#endif
