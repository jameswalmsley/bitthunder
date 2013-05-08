/**
 *	Zynq Device Configuration driver.
 *
 *	Exposes a file/IO (char device) interface to the devcfg hardware.
 *
 **/

#include <bitthunder.h>
#include "devcfg.h"
#include "slcr.h"

BT_DEF_MODULE_NAME				("Zynq Device Configuration")
BT_DEF_MODULE_DESCRIPTION		("Provides filesystem access to the devcfg hardware")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	volatile ZYNQ_DEVCFG_REGS	*pRegs;
};

static BT_BOOL g_bInUse = BT_FALSE;

static BT_ERROR devcfg_cleanup(BT_HANDLE h) {
	g_bInUse = BT_FALSE;
	return BT_ERR_NONE;
}

/**
 *	This assumes a single write request will be generated.
 **/
static BT_u32 devcfg_write(BT_HANDLE hDevcfg, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	// Wait for PCFG_INIT bit to be high.

	// Enable level shifters
	ZYNQ_SLCR->LVL_SHFTR_EN = 0x0000000A;	// Enable PS to PL level shifting.

	// Reset the PL

	hDevcfg->pRegs->CTRL |= CTRL_PCFG_PROG_B;	// Setting PCFG_PROGB signal to high

	BT_ThreadYield();							// A small delay.

	hDevcfg->pRegs->CTRL &= ~CTRL_PCFG_PROG_B;	// Setting PCFG_PROGB signal to low

	while(hDevcfg->pRegs->STATUS & STATUS_PCFG_INIT) {	// Wait for PL for reset
		BT_ThreadYield();
	}

	hDevcfg->pRegs->CTRL |= CTRL_PCFG_PROG_B;

	while(!(hDevcfg->pRegs->STATUS & STATUS_PCFG_INIT)) {	// Wait for PL for status set
		BT_ThreadYield();
	}

	hDevcfg->pRegs->INT_STS = 0xFFFFFFFF;

	hDevcfg->pRegs->MCTRL &= ~MCTRL_PCAP_LPBK;

	hDevcfg->pRegs->DMA_SRC_ADDR = (BT_u32 ) pBuffer | 1;
	hDevcfg->pRegs->DMA_DST_ADDR = 0xFFFFFFFF;

	hDevcfg->pRegs->DMA_SRC_LEN = (ulSize/4);
	hDevcfg->pRegs->DMA_DST_LEN = (ulSize/4);

	while(!(hDevcfg->pRegs->INT_STS & INT_STS_DMA_DONE_INT)) {
		BT_ThreadYield();
	}

	hDevcfg->pRegs->INT_STS = INT_STS_DMA_DONE_INT;	// Clear FPGA_DONE status.

	return ulSize;
}


static BT_u32 devcfg_read(BT_HANDLE hDevcfg, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	return 0;
}

static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= devcfg_read,
	.pfnWrite 	= devcfg_write,
};

static BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = devcfg_cleanup,
	.eType = BT_HANDLE_T_FILE,
	.oIfs = {
		.pFileIF = &oFileOperations,
	},
};

static BT_HANDLE devcfg_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	if(g_bInUse) {
		Error = BT_ERR_GENERIC;
		goto err_set_out;
	}

	g_bInUse = BT_TRUE;

	BT_HANDLE hDevcfg = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hDevcfg) {
		goto err_set_out;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hDevcfg->pRegs = (ZYNQ_DEVCFG_REGS *) pResource->ulStart;

	hDevcfg->pRegs->UNLOCK = 0x757BDF0D;				// Unlock the DEVCFG interface.

	hDevcfg->pRegs->INT_STS = 0xFFFFFFFF;				// Clear all interrupt status signals.

	hDevcfg->pRegs->CTRL |= CTRL_PCAP_MODE;				// Enable PCAP transfer mode.
	hDevcfg->pRegs->CTRL |= CTRL_PCAP_PR;				// Select PCAP for reconfiguration, (disables ICAP).
	hDevcfg->pRegs->CTRL &= ~CTRL_QUARTER_PCAP_RATE;	// Set full bandwidth PCAP loading rate.

	if(pError) {
		*pError = BT_ERR_NONE;
	}

	return hDevcfg;

err_free_out:
	BT_kFree(hDevcfg);

err_set_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF devcfg_driver = {
	.name 		= "zynq,devcfg",
	.pfnProbe	= devcfg_probe,
};
