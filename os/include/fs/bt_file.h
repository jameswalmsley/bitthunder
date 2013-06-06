#ifndef _BT_FILE_H_
#define _BT_FILE_H_



BT_u32 BT_Read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
BT_u32 BT_Write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
BT_s32 BT_GetC(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError);
BT_ERROR BT_PutC(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData);
BT_ERROR BT_Seek(BT_HANDLE hFile, BT_u64 ulOffset);
BT_u64 BT_Tell(BT_HANDLE hFile, BT_ERROR *pError);
BT_s32 BT_GetS(BT_HANDLE hFile, BT_u32 ulSize, BT_i8 *s);




#endif
