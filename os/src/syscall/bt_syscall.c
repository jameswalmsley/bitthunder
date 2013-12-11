/**
 *	BitThunder System Call Dispatcher.
 *
 **/

#include <bt_config.h>
#include <process/bt_threads.h>
#include <process/bt_process.h>
#include <bt_types.h>

/*bt_register_t bt_syscall_handler(bt_register_t a1, bt_register_t a2, bt_register_t a3, bt_register_t a4,
  bt_register_t a5, bt_register_t a6, bt_register_t nr);*/

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

static long getpid(bt_register_t a1, bt_register_t a2, bt_register_t a3, bt_register_t a4) {
	return curtask->pid;
}

static long ksleep(BT_u32 ticks) {
	BT_ThreadSleep(ticks);
}

static long open(const char *path, BT_u32 flags, BT_ERROR *pError, bt_register_t a4) {
	return BT_Open(path, BT_GetModeFlags("wb"), pError);
}

static long write(BT_HANDLE h, BT_u32 len, void *pBuffer, BT_ERROR *pError) {
	return BT_Write(h, 0, len, pBuffer, pError);
}

static long close(BT_HANDLE h) {
	BT_CloseHandle(h);
}

static long printk(const char *path) {
	BT_kPrint("PID %d says: %s", curtask->pid, path);
}

static long gpio(int flag, int state) {
	BT_GpioSet(flag, state);
}

static const struct syscall_entry syscall_table[] = {
	/*		0 */	SYSCALL(0, getpid),
	/*		1 */	SYSCALL(0, open),
	/*		2 */ 	SYSCALL(1, printk),
	/*		3 */	SYSCALL(1, ksleep),
	/*		4 */	SYSCALL(4, write),
	/*		5 */	SYSCALL(5, close),
	/*		6 */	SYSCALL(2, gpio),
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
