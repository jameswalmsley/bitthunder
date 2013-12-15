#include <bitthunder.h>

long bt_sys_sleep(BT_u32 ticks) {
	BT_ThreadSleep(ticks);
	return 0;
}
