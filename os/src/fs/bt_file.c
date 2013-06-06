/**
 *	BitThunder File Access API.
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;

};

static BT_BOOL isHandleValid(BT_HANDLE h, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_BOOL	 result = BT_FALSE;

	if(!h) {
		Error = BT_ERR_NULL_POINTER;
		goto done;
	}

	result = BT_TRUE;

done:
	if(pError) {
		*pError = Error;
	}

	return result;
}

BT_u32 BT_Read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnRead(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_u32 BT_Write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnWrite(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_s32 BT_GetC(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError) {
	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnGetC(hFile, ulFlags, pError);
}

BT_ERROR BT_PutC(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {

	BT_ERROR Error;

	if(!isHandleValid(hFile, &Error)) {
		return Error;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnPutC(hFile, ulFlags, cData);
}

BT_ERROR BT_Seek(BT_HANDLE hFile, BT_u64 ulOffset) {

	BT_ERROR Error;

	if(!isHandleValid(hFile, &Error)) {
		return 0;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnSeek(hFile, ulOffset);
}

BT_u64 BT_Tell(BT_HANDLE hFile, BT_ERROR *pError) {

	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	return hFile->h.pIf->oIfs.pFileIF->pfnTell(hFile);
}
