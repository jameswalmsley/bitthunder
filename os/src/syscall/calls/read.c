#include <bitthunder.h>

long bt_sys_read(BT_HANDLE h, BT_u32 len, void *pBuffer, BT_ERROR *pError) {
	return BT_Read(h, 0, len, pBuffer, pError);
}
