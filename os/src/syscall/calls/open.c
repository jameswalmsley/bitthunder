#include <bitthunder.h>

long bt_sys_open(const BT_i8 *path, BT_u32 flags, BT_ERROR *pError) {
	return (long) BT_Open(path, BT_GetModeFlags("wb"), pError);
}
