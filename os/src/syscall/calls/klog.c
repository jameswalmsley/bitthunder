#include <bitthunder.h>

long bt_sys_klog(const BT_i8 *path) {
	BT_kPrint("PID %d says: %s", curtask->pid, path);
	return 0;
}
