#ifndef _BT_VOLUME_H_
#define _BT_VOLUME_H_

typedef enum _BT_VOLUME_TYPE {
	BT_VOLUME_NORMAL,
	BT_VOLUME_PARTITION,
} BT_VOLUME_TYPE;

typedef struct _BT_VOLUME_DESCRIPTOR {
    BT_HANDLE_HEADER        h;
    struct bt_list_head     item;
    struct bt_devfs_node    node;
    BT_VOLUME_TYPE 			eType;
    BT_u32                  ulTotalBlocks;
    BT_BLKDEV_DESCRIPTOR   *blkdev;
    BT_u32                  ulReferenceCount;
    void                   *kMutex;
} BT_VOLUME_DESCRIPTOR;


BT_ERROR 	BT_EnumerateVolumes	(BT_BLKDEV_DESCRIPTOR *blk);
BT_s32 		BT_VolumeRead		(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer);
BT_s32 		BT_VolumeWrite		(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer);
BT_ERROR    BT_GetVolumeGeometry(BT_HANDLE hVolume, BT_BLOCK_GEOMETRY *pGeometry);

#endif
