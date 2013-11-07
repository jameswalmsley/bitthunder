#ifndef _BT_FS_H_
#define _BT_FS_H_

typedef struct _BT_FILESYSTEM {
	struct bt_list_head item;
	BT_HANDLE	 hFS;
} BT_FILESYSTEM;

typedef struct _BT_MOUNTPOINT {
	struct bt_list_head item;
	BT_HANDLE 		hMount;
	BT_i8 		   *szpPath;
	BT_FILESYSTEM  *pFS;
} BT_MOUNTPOINT;

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS);

BT_HANDLE BT_Open(const BT_i8 *szpPath, BT_i8 *mode, BT_ERROR *pError);
BT_ERROR BT_Mount(const BT_i8 *src, const BT_i8 *target, const BT_i8 *filesystem, BT_u32 mountflags, const void *data);
BT_ERROR BT_MkDir(const BT_i8 *szpPath);
BT_HANDLE BT_OpenDir(const BT_i8 *szpPath, BT_ERROR *pError);
BT_HANDLE BT_GetInode(const BT_i8 *szpPath, BT_ERROR *pError);
BT_ERROR BT_Remove(const BT_i8 *szpPath);
BT_ERROR BT_Rename(const BT_i8 *szpPathA, const BT_i8 *szpPathB);
BT_ERROR BT_GetCwd(BT_i8 *buf, BT_u32 len);
BT_ERROR BT_ChDir(const BT_i8 *path);

#endif
