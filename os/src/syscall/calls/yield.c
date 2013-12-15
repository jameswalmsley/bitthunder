#include <bitthunder.h>

long bt_sys_yield(void) {
	BT_ThreadYield();
	return 0;
}
