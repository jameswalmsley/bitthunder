#ifndef _BT_DEV_IF_MTD_H_
#define _BT_DEV_IF_MTD_H_

#include "bt_types.h"

typedef struct _BT_MTD_ERASE_INFO {
	BT_u64			 addr;
	BT_u32			 len;
	BT_u64			 fail_addr;
	BT_u64			 time;
	BT_u32			 retries;
	BT_u32			 dev;
	BT_u32			 cell;
	void (*callback) (BT_HANDLE hFlash, struct _BT_MTD_ERASE_INFO *self);
	BT_u32			 priv;
	BT_u8			 state;
	struct _BT_MTD_ERASE_INFO 	*next;
} BT_MTD_ERASE_INFO;

typedef struct {
	BT_ERROR	(*pfnErase)		(BT_HANDLE hMtd, BT_MTD_ERASE_INFO *instr);
	BT_s32		(*pfnRead)		(BT_HANDLE hMtd, BT_u64 address, BT_u32 len, BT_u8 *buf);
	BT_s32		(*pfnWrite)		(BT_HANDLE hMtd, BT_u64 address, BT_u32 len, const BT_u8 *buf);
} BT_DEV_IF_MTD;

#endif
