/**
 *	BitThunder Volume and Partition Manager
 *
 *
 **/

#include <bitthunder.h>


BT_DEF_MODULE_NAME			("Volume and Partition Manager")
BT_DEF_MODULE_DESCRIOPTION	("Volume manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef enum _BT_VOLUME_TYPE {
	BT_VOLUME_NORMAL,
	BT_VOLUME_PARTITION,
} BT_VOLUME_TYPE;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	BT_LIST_ITEM		oItem;
	BT_VOLUME_TYPE 		eType;
	BT_HANDLE			hBlockDevice;
	BT_u32				ulTotalBlocks;
	BT_u32				ulBlockSize;
};

struct _BT_PARTITION {
	struct _BT_OPAQUE_HANDLE oVolume;
	BT_u32	ulBaseAddress;
}

static BT_LIST g_oVolumes 		= {0};
static BT_LIST g_oPartitions 	= {0};





BT_ERROR BT_EnumerateVolumes(BT_HANDLE hBlockDevice) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_BLOCK_GEOMETRY oGeometry;

	if(hBlockDevice->h.pIf->eType != BT_HANDLE_T_BLOCK) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	Error = BT_GetBlockGeometry(hBlockDevice, &oGeometry);
	if(Error) {
		goto err_out;
	}

	BT_u8 *pMBR = BT_kMalloc(oGeometry.ulBlockSize);
	if(!pMBR) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	if(BT_BlockRead(hBlockDevice, 0, 1, pMBR, &Error) != 1) {
		goto err_free_out;
	}

err_free_out:
	BT_kFree(pMBR);

err_out:
	return Error;
}

static BT_ERROR bt_volume_manager_init() {
	BT_ListInit(&g_oVolumes);
	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_volume_manager_init,
};
