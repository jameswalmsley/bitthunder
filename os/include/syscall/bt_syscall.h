#ifndef _BT_SYSCALL_H_
#define _BT_SYSCALL_H_

long bt_sys_yield(void);
long bt_sys_getpid(void);
long bt_sys_open(const BT_i8 *path, BT_u32 flags, BT_ERROR *pError);
long bt_sys_close(BT_HANDLE h);
long bt_sys_read(BT_HANDLE h, BT_u32 len, void *pBuffer, BT_ERROR *pError);
long bt_sys_write(BT_HANDLE h, BT_u32 len, const void *pBuffer, BT_ERROR *pError);
long bt_sys_klog(const BT_i8 *path);
long bt_sys_sleep(BT_u32 ticks);
long bt_sys_gpioset(BT_u32 flag, BT_BOOL state);
long bt_sys_gettimeofday(struct bt_timeval *tv, struct bt_timezone *tz);
long bt_sys_settimeofday(struct bt_timeval *tv, struct bt_timezone *tz);

#define BT_SYS_yield		0
#define BT_SYS_getpid		1
#define BT_SYS_open			2
#define BT_SYS_close		3
#define BT_SYS_read			4
#define BT_SYS_write		5
#define BT_SYS_klog			6
#define BT_SYS_sleep		7
#define BT_SYS_gpioset		8
#define BT_SYS_gettimeofday	9
#define BT_SYS_settimeofday	10

#endif
