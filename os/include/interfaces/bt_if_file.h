#ifndef _BT_IF_FILE_H_
#define _BT_IF_FILE_H_

/**
 *	@brief		Defines the interface for reading or writing from FILES or FILE-like devices/modules.
 *
 *	Any device that performs file like (streamable or seekable) I/O should implement this interface.
 *	For block devices see the BT_IF_BLOCK interface.
 *
 *	@pfnRead	[REQUIRED]	Reads specified number of bytes from hFile into pBuffer, using behaviour described in ulFlags.
 *
 *	@pfnWrite	[REQUIRED]	Writes specified number of bytes into hFile from pBuffer, using behaviour described in ulFlags.
 *
 *	@pfnGetC	[OVERRIDE]	Overrides the default GetC implementation (which just calls pfnRead) in case a more efficient
 *							implementation can be provided.
 *
 *	@pfnPutC	[OVERRIDE]	Overrides the default PutC implementation (which just calls pfnWrite) in case a more efficient
 *							implementation can be provided.
 *
 *	@pfnSeek	[OPTIONAL]	Seeks to the absolute specified offset from the origin of the file.
 *
 *	@pfnTell	[OPTIONAL]	Gives the absolute current position of the file from the file's origin.
 *
 *	@pfnFlush	[OPTIONAL]	If possible, any file handle should allow a flush data.
 *
 *	@ulSupported			A mask of FILE flags supported.
 *
 **/
typedef struct _BT_IF_FILE {
	BT_u32 		(*pfnRead)	(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
	BT_u32 		(*pfnWrite)	(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError);
	BT_s32 		(*pfnGetC)	(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError);
	BT_ERROR	(*pfnPutC)	(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData);
	BT_ERROR	(*pfnSeek)	(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence);
	BT_u64		(*pfnTell)	(BT_HANDLE hFile);
	BT_ERROR	(*pfnFlush)	(BT_HANDLE hFile);
	BT_u32		ulSupported;
} BT_IF_FILE;




#endif
