#include <bitthunder.h>

long bt_sys_gpioset(BT_u32 flag, BT_BOOL state) {
	BT_GpioSet(flag, state);
	return 0;
}
