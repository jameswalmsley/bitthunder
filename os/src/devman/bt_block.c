/**
 *	Block Device Manager for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>

BT_DEF_MODULE_NAME			("Block-device Manager")
BT_DEF_MODULE_DESCRIPTION	("Block Device manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_BLKDEV_DESCRIPTOR 	b;
};

#ifdef BT_CONFIG_BLOCK_SCHEDULER
static BT_LIST_HEAD(g_requests);

static void *g_list_mutex = NULL;
static struct bt_thread *g_block_thread;
static BT_EVGROUP_T g_event_group = NULL;

struct bt_block_request {
	struct bt_list_head 	item;
	BT_HANDLE 				hBlock;
	BT_BOOL					write_nRead;
	BT_u32					ulAddress;
	BT_u32 					ulBlocks;
	void 				   *pBuffer;
	volatile BT_BOOL		bDone;
	BT_s32					retval;
};
#endif

static BT_LIST_HEAD(g_block_devices);

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {

	BT_BLKDEV_DESCRIPTOR *blk = bt_container_of(node, BT_BLKDEV_DESCRIPTOR, node);
	if(!blk->ulReferenceCount) {
		blk->ulReferenceCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &blk->h);
		return (BT_HANDLE) blk;
	}

	if(pError) {
		*pError = BT_ERR_GENERIC;
	}

	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

static BT_BOOL isHandleValid(BT_HANDLE hBlock) {
	if(hBlock && BT_HANDLE_TYPE(hBlock) == BT_HANDLE_T_BLOCK) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

#ifdef BT_CONFIG_BLOCK_SCHEDULER
static void bt_block_schedule(struct bt_block_request *req) {
	BT_kMutexPend(g_list_mutex, BT_INFINITE_TIMEOUT);
	{
		bt_list_add(&req->item, &g_requests);
	}
	BT_kMutexRelease(g_list_mutex);

	BT_kTaskNotify(g_block_thread, 0, BT_NOTIFY_INCREMENT);
}

static void bt_block_exec_request(BT_HANDLE hBlock, struct bt_block_request *req) {
	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	bt_block_schedule(req);

	while(req->bDone != BT_TRUE) {
		BT_kEventGroupWaitBits(g_event_group, 1, BT_TRUE, BT_FALSE, BT_INFINITE_TIMEOUT);
	}
}
#endif

BT_s32 BT_BlockRead(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {

	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

#ifndef BT_CONFIG_BLOCK_SCHEDULER

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->b.h.pIf->oIfs.pDevIF->pBlockIF;

	BT_kMutexPend(blkdev->kMutex, BT_INFINITE_TIMEOUT);
	BT_s32 ret = pOps->pfnReadBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer);
	BT_kMutexRelease(blkdev->kMutex);

	return ret;
#else
	struct bt_block_request req;
	req.hBlock = hBlock;
	req.write_nRead = BT_FALSE;
	req.ulAddress = ulAddress;
	req.ulBlocks = ulBlocks;
	req.pBuffer = pBuffer;
	req.bDone = BT_FALSE;

	bt_block_exec_request(hBlock, &req);	// Block until operation complete.

	return req.retval;

#endif
}
BT_EXPORT_SYMBOL(BT_BlockRead);

BT_s32 BT_BlockWrite(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {

	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

#ifndef BT_CONFIG_BLOCK_SCHEDULER

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->b.h.pIf->oIfs.pDevIF->pBlockIF;

	BT_kMutexPend(blkdev->kMutex, BT_INFINITE_TIMEOUT);
	BT_s32 ret = pOps->pfnWriteBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer);
	BT_kMutexRelease(blkdev->kMutex);

	return ret;

#else
	struct bt_block_request req;
	req.hBlock = hBlock;
	req.write_nRead = BT_TRUE;
	req.ulAddress = ulAddress;
	req.ulBlocks = ulBlocks;
	req.pBuffer = pBuffer;
	req.bDone = BT_FALSE;

	bt_block_exec_request(hBlock, &req);

	return req.retval;
#endif
}
BT_EXPORT_SYMBOL(BT_BlockWrite);

BT_ERROR BT_GetBlockGeometry(BT_HANDLE hBlock, BT_BLOCK_GEOMETRY *pGeometry) {

	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;
	if(pGeometry) {
		*pGeometry = blkdev->oGeometry;
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_GetBlockGeometry);

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_BLKDEV_DESCRIPTOR *pDescriptor) {

	BT_ERROR Error;

	pDescriptor->h.pIf = &oHandleInterface;
	pDescriptor->hBlkDev = hDevice;
	pDescriptor->kMutex = BT_kMutexCreate();
	bt_list_add(&pDescriptor->item, &g_block_devices);

	BT_LIST_INIT_HEAD(&pDescriptor->volumes);

	pDescriptor->node.pOps = &oDevfsOps;

	Error = BT_DeviceRegister(&pDescriptor->node, szpName);

	BT_kPrint("Block device: %s registered, enumerating partitions.", szpName);

	BT_EnumerateVolumes((BT_HANDLE) pDescriptor);

	return Error;
}
BT_EXPORT_SYMBOL(BT_RegisterBlockDevice);

/*BT_HANDLE BT_BlockGetInode(BT_HANDLE hDevice) {
	if(!isHandleValid(hDevice)) {
		return NULL;
	}

	return hDevice->hInode;
	}*/

BT_ERROR BT_UnregisterBlockDevice(BT_HANDLE hDevice) {
	// find the block device entry that owns this handle

	// notify any users that this block device is now invalid!

	// free memory.
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_UnregisterBlockDevice);

static BT_ERROR bt_blockdev_cleanup(BT_HANDLE hBlockDev) {

	BT_BLKDEV_DESCRIPTOR *blk = (BT_BLKDEV_DESCRIPTOR *) hBlockDev;

	if(blk->ulReferenceCount) {
		blk->ulReferenceCount -= 1;
	}

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_BLOCK,
	.pfnCleanup = bt_blockdev_cleanup,
};

#ifdef BT_CONFIG_BLOCK_SCHEDULER
static BT_ERROR block_scheduler(BT_HANDLE hThread, void *pParam) {

	BT_kDebug("Started the block scheduler (basic)");

	while(1) {
		BT_kTaskNotifyTake(BT_FALSE, BT_INFINITE_TIMEOUT);

		BT_kMutexPend(g_list_mutex, BT_INFINITE_TIMEOUT);
		{
			struct bt_list_head *pos, *next;
			bt_list_for_each_safe(pos, next, &g_requests) {
				struct bt_block_request *req = (struct bt_block_request *) pos;
				BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) req->hBlock;
				const BT_IF_BLOCK *pOps = blkdev->hBlkDev->b.h.pIf->oIfs.pDevIF->pBlockIF;

				BT_kMutexRelease(g_list_mutex);
				{
					if(req->write_nRead) {
						req->retval = pOps->pfnWriteBlocks(blkdev->hBlkDev, req->ulAddress, req->ulBlocks, req->pBuffer);
					} else {
						req->retval = pOps->pfnReadBlocks(blkdev->hBlkDev, req->ulAddress, req->ulBlocks, req->pBuffer);
					}
				}
				BT_kMutexPend(g_list_mutex, BT_INFINITE_TIMEOUT);

				bt_list_del(&req->item);

				req->bDone = BT_TRUE;
				BT_kEventGroupSetBits(g_event_group, 1);
			}
		}
		BT_kMutexRelease(g_list_mutex);
	}
}

BT_ERROR bt_block_init() {
	g_list_mutex = BT_kMutexCreate();
	g_event_group = BT_kEventGroupCreate();

	BT_ERROR Error = BT_ERR_NONE;

	BT_THREAD_CONFIG oConfig;
	oConfig.ulStackDepth 	= 128;
	oConfig.ulPriority 		= BT_CONFIG_INTERRUPTS_SOFTIRQ_PRIORITY;

	BT_HANDLE hThread = BT_CreateThread(block_scheduler, &oConfig, &Error);
	g_block_thread = BT_GetThreadDescripter(hThread);

	return Error;
}

BT_MODULE_INIT_0_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_block_init,
};
#endif
