#include <bitthunder.h>

long bt_sys_gettimeofday(struct bt_timeval *tv, struct bt_timezone *tz) {
	return bt_gettimeofday(tv, tz);
}

long bt_sys_settimeofday(struct bt_timeval *tv, struct bt_timezone *tz) {
	return bt_settimeofday(tv, tz);
}
