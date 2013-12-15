/**
 *	BitThunder System Call Dispatcher.
 *
 **/

#include <bitthunder.h>
#include <bt_config.h>
#include <process/bt_threads.h>
#include <process/bt_process.h>
#include <bt_types.h>
#include <syscall/bt_syscall.h>

/**
 *	Syscall can have upto 6 arguments!
 **/
typedef long (*syscall_fn)(bt_register_t, bt_register_t, bt_register_t, bt_register_t);


struct syscall_entry {
#ifdef BT_SYSCALL_DEBUG
	BT_u32 			nargs;
	const BT_i8    *name;
#endif
	syscall_fn		syscall;
};

#ifdef BT_SYSCALL_DEBUG
#define SYSCALL(n, fn)	{n, #fn, (syscall_fn)(fn)}
#else
#define SYSCALL(n, fn)	{(syscall_fn)(fn)}
#endif

static const struct syscall_entry syscall_table[] = {
	/*		0 */	SYSCALL(0, bt_sys_yield),
	/*		1 */	SYSCALL(0, bt_sys_getpid),
	/*		2 */	SYSCALL(3, bt_sys_open),
	/*		3 */	SYSCALL(1, bt_sys_close),
	/*		4 */	SYSCALL(4, bt_sys_read),
	/*		5 */	SYSCALL(4, bt_sys_write),
	/*		6 */ 	SYSCALL(1, bt_sys_klog),
	/*		7 */	SYSCALL(1, bt_sys_sleep),
	/*		8 */	SYSCALL(2, bt_sys_gpioset),
};

#define SYSCALL_TOTAL	(BT_u32) (sizeof(syscall_table)/sizeof(struct syscall_entry))

bt_register_t bt_syscall_handler(bt_register_t a1, bt_register_t a2, bt_register_t a3, bt_register_t a4,
								 bt_register_t nr) {

	bt_register_t retval = -1;
	const struct syscall_entry *syscall;

	if(nr < SYSCALL_TOTAL) {
		syscall = &syscall_table[nr];
		retval = syscall->syscall(a1, a2, a3, a4);
	}

	return retval;
}
