#ifndef _BT_FS_H_
#define _BT_FS_H_

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS);

BT_HANDLE BT_Open(const BT_i8 *szpPath, BT_i8 *mode, BT_ERROR *pError);
BT_ERROR BT_Mount(const BT_i8 *src, const BT_i8 *target, const BT_i8 *filesystem, BT_u32 mountflags, const void *data);
BT_ERROR BT_MkDir(const BT_i8 *szpPath);
BT_HANDLE BT_OpenDir(const BT_i8 *szpPath, BT_ERROR *pError);
BT_HANDLE BT_GetInode(const BT_i8 *szpPath, BT_ERROR *pError);
BT_ERROR BT_Remove(const BT_i8 *szpPath);
BT_ERROR BT_Rename(const BT_i8 *szpPathA, const BT_i8 *szpPathB);

#endif
