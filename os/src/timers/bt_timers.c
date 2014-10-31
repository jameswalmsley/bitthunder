/**
 *	Timer API
 *
 **/

#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_TIMER_CONTROLLER {
	BT_HANDLE 	hTimers;
	BT_u32		ulBaseTimer;
	BT_u32		ulTotalTimers;
} BT_TIMER_CONTROLLER;

static BT_HANDLE g_hTimer = NULL;
static BT_HANDLE g_gTimer = NULL;

static const BT_DEV_IF_SYSTIMER *g_Ops 	= NULL;
static const BT_DEV_IF_GTIMER 	*g_gOps = NULL;

BT_ERROR BT_SetSystemTimerHandle(BT_HANDLE hTimer) {
	if(!g_hTimer) {
		g_hTimer = hTimer;
		g_Ops = BT_IF_SYSTIMER_OPS(hTimer);
		return BT_ERR_NONE;
	}

	return BT_ERR_GENERIC;
}
BT_EXPORT_SYMBOL(BT_SetSystemTimerHandle);

BT_ERROR BT_SetGlobalTimerHandle(BT_HANDLE hTimer) {
	if(!g_gTimer) {
		g_gTimer = hTimer;
		g_gOps = BT_IF_GTIMER_OPS(hTimer);
		return BT_ERR_NONE;
	}
	return BT_ERR_GENERIC;
}
BT_EXPORT_SYMBOL(BT_SetGlobalTimerHandle);

BT_ERROR BT_StopSystemTimer() {
	return g_Ops->pfnStop(g_hTimer);
}
BT_EXPORT_SYMBOL(BT_StopSystemTimer);

BT_u32 BT_GetSystemTimerOffset() {
	BT_ERROR Error;

	if(g_hTimer) {
		BT_u32 rate = g_Ops->pfnGetClockRate(g_hTimer, &Error);
		return g_Ops->pfnGetOffset(g_hTimer, &Error) / (rate / 1000000);
	}

	return 0;
}
BT_EXPORT_SYMBOL(BT_GetSystemTimerOffset);

BT_u64 BT_GetGlobalTimer() {

	BT_ERROR Error;

	if(g_gTimer) {
		return g_gOps->pfnGetValue(g_gTimer, &Error);
	}
	else {
		return (((BT_u64)BT_GetKernelTime()) << 32);
		//return (((BT_u64)BT_kTickCount()) << 32);
	}

	return 0;
}
BT_EXPORT_SYMBOL(BT_GetGlobalTimer);

BT_u32 BT_GetGlobalTimerRate() {

	BT_ERROR Error;

	if(g_gTimer) {
		return g_gOps->pfnGetClockRate(g_gTimer, &Error);
	}

	return 0;
}
BT_EXPORT_SYMBOL(BT_GetGlobalTimerRate);

BT_u32 BT_GetKernelTime() {
	BT_u32 ulOldTick, ulNewTick;
	BT_u32 us;
	do {
		ulOldTick = BT_kTickCount();
		us = BT_GetSystemTimerOffset();
		ulNewTick = BT_kTickCount();
	} while (ulNewTick != ulOldTick);

	return ((ulOldTick * 1000) + us);
}
BT_EXPORT_SYMBOL(BT_GetKernelTime);

BT_u32 BT_GetKernelTick() {
	return BT_kTickCount();
}
BT_EXPORT_SYMBOL(BT_GetKernelTick);
