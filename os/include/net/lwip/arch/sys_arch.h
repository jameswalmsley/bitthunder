#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#include "bitthunder.h"


typedef BT_HANDLE sys_sem_t;
typedef BT_HANDLE sys_mutex_t;
typedef BT_HANDLE sys_mbox_t;
typedef BT_HANDLE sys_thread_t;

#define SYS_MBOX_NULL					( ( sys_mbox_t ) NULL )
#define SYS_SEM_NULL					( ( sys_sem_t ) NULL )
#define SYS_DEFAULT_THREAD_STACK_DEPTH	128

typedef BT_s32				sys_prot_t;


#define sys_mbox_valid( x ) ( ( ( *x ) == NULL) ? BT_FALSE : BT_TRUE )
#define sys_mbox_set_invalid( x ) ( ( *x ) = NULL )
#define sys_sem_valid( x ) ( ( ( *x ) == NULL) ? BT_FALSE : BT_TRUE )
#define sys_sem_set_invalid( x ) ( ( *x ) = NULL )


#endif /* __ARCH_SYS_ARCH_H__ */

