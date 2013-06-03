#ifndef _BT_TIMERS_H_
#define _BT_TIMERS_H_

#include <bitthunder.h>

BT_ERROR BT_SetSystemTimerHandle(BT_HANDLE hTimer);

BT_u32 BT_GetSystemTimerOffset(void);
BT_ERROR BT_StopSystemTimer();

BT_u32 BT_GetKernelTime(void);
BT_u32 BT_GetKernelTick(void);




#endif
