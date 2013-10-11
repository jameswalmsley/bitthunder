#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_ERROR BT_ReadDir(BT_HANDLE hDir, BT_DIRENT *pDirent) {
	if(!hDir && hDir->h.pIf->eType != BT_HANDLE_T_DIRECTORY) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	return hDir->h.pIf->oIfs.pDirIF->pfnReadDir(hDir, pDirent);
}
