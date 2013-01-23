#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_BOOL chardevIfSupported(BT_HANDLE hDevice) {
	if(!hDevice || !BT_IF_DEVICE(hDevice) || !BT_IF_CHARDEV_OPS(hDevice)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

BT_ERROR BT_CharDeviceRead(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	if(!chardevIfSupported(hDevice)) {
		return (BT_ERROR) -1;
	}
	return BT_IF_CHARDEV_OPS(hDevice)->pfnRead(hDevice, ulFlags, ulSize, pucDest);
}

BT_ERROR BT_CharDeviceWrite(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource) {
	if(!chardevIfSupported(hDevice)) {
		return (BT_ERROR) -1;
	}

	return BT_IF_CHARDEV_OPS(hDevice)->pfnWrite(hDevice, ulFlags, ulSize, pucSource);
}
