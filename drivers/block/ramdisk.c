#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME			("Ramdisk")
BT_DEF_MODULE_DESCRIPTION	("Simple ram based block disk")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	BT_BLKDEV_DESCRIPTOR oDescriptor;
	void *buffer;
};

static BT_u32 ramdisk_blockread(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {
	BT_u8 *p = (BT_u8 *) hBlock->buffer;
	p += (hBlock->oDescriptor.oGeometry.ulBlockSize * ulBlock);
	memcpy(pBuffer, p, hBlock->oDescriptor.oGeometry.ulBlockSize * ulCount);
	return ulCount;
}

static BT_u32 ramdisk_blockwrite(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {
	BT_u8 *p = (BT_u8 *) hBlock->buffer;
	p += (hBlock->oDescriptor.oGeometry.ulBlockSize * ulBlock);
	memcpy(hBlock->buffer, pBuffer, hBlock->oDescriptor.oGeometry.ulBlockSize * ulCount);
	return ulCount;
}

static const BT_IF_BLOCK ramdisk_blockdev_interface = {
	ramdisk_blockread,
	ramdisk_blockwrite,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pBlockIF = &ramdisk_blockdev_interface,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_DEVICE,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static BT_HANDLE ramdisk_probe(const BT_DEVICE *pDevice, BT_ERROR *pError) {
	BT_HANDLE hBlock = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_INTEGER, 0);
	hBlock->oDescriptor.oGeometry.ulBlockSize = pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_INTEGER, 1);
	hBlock->oDescriptor.oGeometry.ulTotalBlocks = pResource->ulStart;

	hBlock->buffer = BT_kMalloc(hBlock->oDescriptor.oGeometry.ulBlockSize * hBlock->oDescriptor.oGeometry.ulTotalBlocks);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_STRING, 0);
	if(pResource) {
		const BT_i8 *image = pResource->szpName;
		BT_HANDLE h = BT_Open(image, BT_GetModeFlags("rb"), NULL);
		BT_Read(h, 0, hBlock->oDescriptor.oGeometry.ulBlockSize * hBlock->oDescriptor.oGeometry.ulTotalBlocks, hBlock->buffer, NULL);
		BT_CloseHandle(h);

	} else {
		memset(hBlock->buffer, 0, hBlock->oDescriptor.oGeometry.ulBlockSize * hBlock->oDescriptor.oGeometry.ulTotalBlocks);
	}

	BT_RegisterBlockDevice(hBlock, "rd0", &hBlock->oDescriptor);

	return hBlock;
}

BT_INTEGRATED_DRIVER_DEF ramdisk_driver = {
	.name = "block,ramdisk",
	.pfnProbe = ramdisk_probe,
};
