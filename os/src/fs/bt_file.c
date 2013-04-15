/**
 *	BitThunder File Access API.
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;

};

BT_u32 BT_Read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	return hFile->h.pIf->oIfs.pFileIF->pfnRead(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_u32 BT_Write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	return hFile->h.pIf->oIfs.pFileIF->pfnWrite(hFile, ulFlags, ulSize, pBuffer, pError);
}

BT_s32 BT_GetC(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError) {
	return hFile->h.pIf->oIfs.pFileIF->pfnGetC(hFile, ulFlags, pError);
}

BT_ERROR BT_PutC(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {
	return hFile->h.pIf->oIfs.pFileIF->pfnPutC(hFile, ulFlags, cData);
}

BT_ERROR BT_Seek(BT_HANDLE hFile, BT_u64 ulOffset) {
	return hFile->h.pIf->oIfs.pFileIF->pfnSeek(hFile, ulOffset);
}

BT_u64 BT_Tell(BT_HANDLE hFile) {
	return hFile->h.pIf->oIfs.pFileIF->pfnTell(hFile);
}
