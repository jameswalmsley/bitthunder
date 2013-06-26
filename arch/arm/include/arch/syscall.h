#ifndef _ARM_SYSCALL_H_
#define _ARM_SYSCALL_H_

#include <syscall/bt_syscall.h>

#define BT_SYSCALL0(name)	\
	.global name;	.align;	\
name##: swi #BT_SYS_##name;	\
    mov pc, lr











#endif
