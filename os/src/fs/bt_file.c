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

	if(!h->h.pIf->pFileIF) {
		Error = BT_ERR_INVALID_HANDLE_TYPE;
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

	return hFile->h.pIf->pFileIF->pfnRead(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_u32 BT_Write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	return hFile->h.pIf->pFileIF->pfnWrite(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_s32 BT_GetC(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	if(hFile->h.pIf->pFileIF->pfnGetC) {
		BT_s32 ret = hFile->h.pIf->pFileIF->pfnGetC(hFile, ulFlags, &Error);
		if(pError) {
			*pError = Error;
		}
		return ret;
	}

	BT_u8 c;
	hFile->h.pIf->pFileIF->pfnRead(hFile, ulFlags, 1, &c, &Error);

	if(pError) {
		*pError = Error;
	}

	return c;
}

BT_ERROR BT_PutC(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {

	BT_ERROR Error;

	if(!isHandleValid(hFile, &Error)) {
		return Error;
	}

	if(hFile->h.pIf->pFileIF->pfnPutC) {
		return hFile->h.pIf->pFileIF->pfnPutC(hFile, ulFlags, cData);
	}

	hFile->h.pIf->pFileIF->pfnWrite(hFile, ulFlags, 1, &cData, &Error);

	return Error;
}

BT_ERROR BT_Seek(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence) {

	BT_ERROR Error;

	if(!isHandleValid(hFile, &Error)) {
		return Error;
	}

	if(!hFile->h.pIf->pFileIF->pfnSeek) {
		return BT_ERR_UNSUPPORTED_INTERFACE;
	}

	return hFile->h.pIf->pFileIF->pfnSeek(hFile, ulOffset);
}

BT_u64 BT_Tell(BT_HANDLE hFile, BT_ERROR *pError) {

	if(!isHandleValid(hFile, pError)) {
		return 0;
	}

	if(!hFile->h.pIf->pFileIF->pfnTell) {
		if(pError) {
			*pError = BT_ERR_UNSUPPORTED_INTERFACE;
		}
		return 0;
	}

	return hFile->h.pIf->pFileIF->pfnTell(hFile);
}



BT_s32 BT_GetS(BT_HANDLE hFile, BT_u32 ulSize, BT_i8 *s) {

	BT_ERROR Error;

	if(!isHandleValid(hFile, &Error)) {
		return (BT_s32) Error;
	}

	BT_i8 *t;
	BT_s32 c;

    t = s;
    while (--ulSize>1 && (c=BT_GetC(hFile, 0, &Error)) != -1 && (c != '\n' && c != '\r')) {
        *s++ = c;
	}

    if (c == '\n' || c == '\r') {
        *s++ = c;
	} else if (ulSize == 1) {
		*s++ = '\n';
    }

    *s = '\0';

    return s - t;
}
