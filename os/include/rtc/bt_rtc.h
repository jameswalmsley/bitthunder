#ifndef _BT_RTC_H_
#define _BT_RTC_H_


struct rtctime {
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

/*
 *	Define the unified API for RTC devices in BitThunder
 */
BT_ERROR BT_RTCSetTime	(BT_HANDLE hRtc, struct rtctime *t);
BT_ERROR BT_RTCGetTime	(BT_HANDLE hRtc, struct rtctime *t);


#endif
