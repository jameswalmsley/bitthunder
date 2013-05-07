/**
 *	Zynq Device Configuration driver.
 *
 *	Exposes a file/IO (char device) interface to the devcfg hardware.
 *
 **/

#include <bitthunder.h>
#include "devcfg.h"

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

	return BT_ERR_NONE;
}

/**
 *	This assumes a single write request will be generated.
 **/
static BT_u32 devcfg_write(BT_HANDLE hDevcfg, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {

	return 0;
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
}

static BT_HANDLE devcfg_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

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

	return hDevcfg;

err_free_out:
	BT_kFree(hDevcfg);

err_set_out:
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF devcfg_driver = {
	.name 		= "zynq,devcfg",
	.pfnProbe	= devcfg_probe,
};
