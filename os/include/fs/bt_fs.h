#ifndef _BT_FS_H_
#define _BT_FS_H_

#define BT_FS_MODE_READ				0x01
#define	BT_FS_MODE_WRITE			0x02
#define BT_FS_MODE_APPEND			0x04
#define	BT_FS_MODE_CREATE			0x08
#define BT_FS_MODE_TRUNCATE			0x10

typedef struct _BT_FILESYSTEM {
	struct bt_list_head item;
	BT_HANDLE	 hFS;
} BT_FILESYSTEM;

typedef struct _BT_MOUNTPOINT {
	struct bt_list_head item;
	BT_HANDLE 		hMount;
	BT_i8 		   *szpPath;
	BT_FILESYSTEM  *pFS;
	BT_HANDLE		hVolume;
} BT_MOUNTPOINT;

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS);

BT_u32 BT_GetModeFlags(const BT_i8 *mode);
BT_HANDLE BT_Open(const BT_i8 *szpPath, BT_u32 mode, BT_ERROR *pError);
#define BT_OpenFile(path, mode_string, perror)	BT_Open(path, BT_GetModeFlags(mode_string), perror)

BT_ERROR BT_Mount(const BT_i8 *src, const BT_i8 *target, const BT_i8 *filesystem, BT_u32 mountflags, const void *data);
BT_ERROR BT_MkDir(const BT_i8 *szpPath);
BT_ERROR BT_RmDir(const BT_i8 *szpPath);
BT_HANDLE BT_OpenDir(const BT_i8 *szpPath, BT_ERROR *pError);
BT_HANDLE BT_GetInode(const BT_i8 *szpPath, BT_ERROR *pError);
BT_ERROR BT_Remove(const BT_i8 *szpPath);
BT_ERROR BT_Unlink(const BT_i8 *szpPath);
BT_ERROR BT_Rename(const BT_i8 *szpPathA, const BT_i8 *szpPathB);
BT_ERROR BT_GetCwd(BT_i8 *buf, BT_u32 len);
BT_ERROR BT_ChDir(const BT_i8 *path);

#endif
