#include <bitthunder.h>

long bt_sys_close(BT_HANDLE h) {
	BT_CloseHandle(h);
	return 0;
}
