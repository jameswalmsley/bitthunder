#ifndef _BT_PARTITION_H_
#define _BT_PARTITION_H_

#define BT_PARTITION_MAX	4

struct bt_part_t {
	BT_u32 ulStartLBA;
	BT_u32 ulSectorCount;
	BT_u32 ucActive : 8, ucPartitionID : 8,  bIsExtended : 1;
};

typedef enum _BT_PARTITION_SIZETYPE {
	BT_PARTITION_SIZE_QUOTA,
	BT_PARTITION_SIZE_PERCENT,
	BT_PARTITION_SIZE_SECTORS,
} BT_PARTITION_SIZETYPE;

typedef struct _BT_PARTITION_PARAMETERS {
	BT_u32 ulHiddenSectors;
	BT_u32 ulInterSpace;
	BT_u32 ulSizes[BT_PARTITION_MAX];
	BT_u32 ulPrimaryCount;
	BT_PARTITION_SIZETYPE eSizeType;
} BT_PARTITION_PARAMETERS;

BT_ERROR BT_Partition(const BT_i8 *device, BT_PARTITION_PARAMETERS *pParams);
BT_ERROR BT_PartitionInfo(BT_HANDLE hBlock, BT_u32 partitionNum, struct bt_part_t *partition);

#endif
