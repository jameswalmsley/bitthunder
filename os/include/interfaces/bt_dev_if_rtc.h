#ifndef _BT_DEV_IF_RTC_H_
#define _BT_DEV_IF_RTC_H_

struct bt_rtc_time {
	BT_s32 tm_sec;
	BT_s32 tm_min;
	BT_s32 tm_hour;
	BT_s32 tm_mday;
	BT_s32 tm_mon;
	BT_s32 tm_year;
	BT_s32 tm_wday;
	BT_s32 tm_yday;
	BT_s32 tm_isdst;
};

typedef struct _BT_DEV_IF_RTC {
	BT_ERROR		(*pfnSetTime)			(BT_HANDLE hRtc, struct bt_rtc_time *t);
	BT_ERROR		(*pfnGetTime)			(BT_HANDLE hRtc, struct bt_rtc_time *t);
} BT_DEV_IF_RTC;

#endif
