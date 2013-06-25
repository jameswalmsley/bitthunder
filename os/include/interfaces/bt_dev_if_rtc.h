#ifndef _BT_DEV_IF_RTC_H_
#define _BT_DEV_IF_RTC_H_

#include "rtc/bt_rtc.h"

typedef struct _BT_DEV_IF_RTC {
	BT_ERROR		(*pfnSetTime)			(BT_HANDLE hRtc, struct rtctime *t);
	BT_ERROR		(*pfnGetTime)			(BT_HANDLE hRtc, struct rtctime *t);
} BT_DEV_IF_RTC;



#endif
