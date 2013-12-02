#ifndef _BT_RTC_H_
#define _BT_RTC_H_

#include <fs/bt_devfs.h>
#include <collections/bt_list.h>

typedef struct _BT_RTC_INFO {
	BT_HANDLE_HEADER h;
	const BT_DEVICE *pDevice;
	BT_HANDLE hRtc;
	struct bt_list_head item;
	struct bt_devfs_node node;
	BT_u32 ulReferenceCount;
} BT_RTC_INFO;

/*
 *	Define the unified API for RTC devices in BitThunder
 */
BT_ERROR BT_RTCRegisterDevice(BT_HANDLE hDevice, BT_RTC_INFO *rtc);
BT_ERROR BT_RTCSetTime	(BT_HANDLE hRtc, struct rtctime *t);
BT_ERROR BT_RTCGetTime	(BT_HANDLE hRtc, struct rtctime *t);

#endif
