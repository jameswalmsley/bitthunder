#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_BOOL chardevIfSupported(BT_HANDLE hDevice) {
	if(!hDevice || !hDevice->h.pIf->oIfs.pDevIF || !hDevice->h.pIf->oIfs.pDevIF->pCharDevIf) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

BT_ERROR BT_CharDeviceRead(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	if(!chardevIfSupported(hDevice)) {
		return (BT_ERROR) -1;
	}
	return hDevice->h.pIf->oIfs.pDevIF->pCharDevIf->pfnRead(hDevice, ulFlags, ulSize, pucDest);
}

BT_ERROR BT_CharDeviceWrite(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource) {
	if(!chardevIfSupported(hDevice)) {
		return (BT_ERROR) -1;
	}
	return hDevice->h.pIf->oIfs.pDevIF->pCharDevIf->pfnWrite(hDevice, ulFlags, ulSize, pucSource);
}
