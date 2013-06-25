#include <bitthunder.h>
#include <rtc/bt_rtc.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isRtcHandle(BT_HANDLE hRtc) {
	if(!hRtc || !BT_IF_DEVICE(hRtc) || (BT_IF_DEVICE_TYPE(hRtc) != BT_DEV_IF_T_RTC)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

/**
 *	@brief	Set Time for the RTC device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_RTCSetTime(BT_HANDLE hRtc, struct rtctime *t) {
	if(!isRtcHandle(hRtc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_RTC_OPS(hRtc)->pfnSetTime(hRtc, t);
}


/**
 *	@brief	Get Time from the RTC device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_RTCGetTime(BT_HANDLE hRtc, struct rtctime *t) {
	if(!isRtcHandle(hRtc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_RTC_OPS(hRtc)->pfnGetTime(hRtc, t);
}
