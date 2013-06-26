/**
 *	BitThunder System Call Dispatcher.
 *
 **/

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
	return 0xDEADBEEF;
}

static long getpid2(bt_register_t a1, bt_register_t a2, bt_register_t a3, bt_register_t a4) {
	return 0xCAFEBABE;
}

static const struct syscall_entry syscall_table[] = {
	/*		0 */	SYSCALL(0, getpid),
	/*		1 */	SYSCALL(0, getpid2),
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
