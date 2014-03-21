#include <bitthunder.h>
#include <rtc/bt_rtc.h>
#include <interfaces/bt_dev_if_rtc.h>

BT_DEF_MODULE_NAME			("RTC device manager")
BT_DEF_MODULE_DESCRIPTION	("Manages RTC devices")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST_HEAD(g_rtc_devices);
static BT_u32 g_total_rtcs = 0;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_BOOL isRtcHandle(BT_HANDLE hRtc) {
	if(BT_HANDLE_TYPE(hRtc) != BT_HANDLE_T_RTC) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {
	BT_RTC_INFO *pInfo = (BT_RTC_INFO *) bt_container_of(node, BT_RTC_INFO, node);
	if(!pInfo->ulReferenceCount) {
		pInfo->ulReferenceCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pInfo->h);
		return (BT_HANDLE) &pInfo->h;
	}

	return NULL;
}

static const BT_DEVFS_OPS rtc_devfs_ops = {
	.pfnOpen = devfs_open,
};

static BT_ERROR bt_rtc_cleanup(BT_HANDLE hRTC) {
	BT_RTC_INFO *pInfo = (BT_RTC_INFO *) hRTC;

	if(pInfo->ulReferenceCount) {
		pInfo->ulReferenceCount -= 1;
	}

	return BT_ERR_NONE;
}

BT_ERROR BT_RTCRegisterDevice(BT_HANDLE hDevice, BT_RTC_INFO *rtc) {

	bt_list_add(&rtc->item, &g_rtc_devices);
	rtc->node.pOps = &rtc_devfs_ops;
	rtc->hRtc = hDevice;

	char name[10];
	bt_sprintf(name, "rtc%d", g_total_rtcs++);

	BT_kPrint("Registering %s as /dev/%s", rtc->pDevice->name, name);

	return BT_DeviceRegister(&rtc->node, name);
}

/**
 *	@brief	Set Time for the RTC device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_RTCSetTime(BT_HANDLE hRtc, struct rtctime *t) {

	BT_RTC_INFO *rtc = (BT_RTC_INFO *) hRtc;

	if(!isRtcHandle(hRtc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_RTC_OPS(rtc->hRtc)->pfnSetTime(rtc->hRtc, t);
}


/**
 *	@brief	Get Time from the RTC device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_RTCGetTime(BT_HANDLE hRtc, struct rtctime *t) {

	BT_RTC_INFO *rtc = (BT_RTC_INFO *) hRtc;

	if(!isRtcHandle(hRtc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_RTC_OPS(rtc->hRtc)->pfnGetTime(rtc->hRtc, t);
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_RTC,
	.pfnCleanup = bt_rtc_cleanup,
};
