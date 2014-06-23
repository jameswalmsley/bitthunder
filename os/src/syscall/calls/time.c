#include <bitthunder.h>
#include <time.h>

long bt_sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
	return bt_gettimeofday(tv, tz);
}

long bt_sys_settimeofday(struct timeval *tv, struct timezone *tz) {
	return bt_settimeofday(tv, tz);
}
