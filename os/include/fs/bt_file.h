#ifndef _BT_FILE_H_
#define _BT_FILE_H_

#define 	BT_FILE_NON_BLOCK	0x00000001	///< Don't block on read-call, return immediately.

#define 	BT_SEEK_SET			0
#define 	BT_SEEK_CUR			1
#define 	BT_SEEK_END			2

BT_u32 		BT_Read	(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
BT_u32 		BT_Write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
BT_s32 		BT_GetC	(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError);
BT_ERROR 	BT_PutC	(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData);
BT_ERROR 	BT_Seek	(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence);
BT_u64 		BT_Tell	(BT_HANDLE hFile, BT_ERROR *pError);
BT_s32 		BT_GetS	(BT_HANDLE hFile, BT_u32 ulSize, BT_i8 *s);
BT_ERROR	BT_Flush(BT_HANDLE hFile);



#endif
