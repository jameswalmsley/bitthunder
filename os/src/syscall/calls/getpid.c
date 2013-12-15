#include <bitthunder.h>

long bt_sys_getpid(void) {
	return curtask->pid;
}
