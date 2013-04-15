#ifndef _BT_FS_H_
#define _BT_FS_H_

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS);

BT_HANDLE BT_Open(BT_i8 *szpPath, BT_i8 *mode, BT_ERROR *pError);
BT_ERROR BT_Mount(BT_HANDLE hVolume, const BT_i8 *szpPath);

#endif
