/**
 *	Timer Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isTimerHandle(BT_HANDLE hTimer) {
	if(!hTimer || !BT_IF_DEVICE(hTimer) || (BT_IF_DEVICE_TYPE(hTimer) != BT_DEV_IF_T_TIMER)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_ERROR BT_TimerStart(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnStart(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerStart);

BT_ERROR BT_TimerStop(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnStop(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerStop);

BT_ERROR BT_TimerEnableInterrupt(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnEnableInterrupt(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerEnableInterrupt);

BT_ERROR BT_TimerDisableInterrupt(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnDisableInterrupt(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerDisableInterrupt);

BT_HANDLE BT_TimerRegisterCallback(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return NULL;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnRegisterCallback(hTimer, pfnCallback, pParam, pError);
}
BT_EXPORT_SYMBOL(BT_TimerRegisterCallback);

BT_ERROR BT_TimerUnregisterCallback(BT_HANDLE hTimer, BT_HANDLE hCallback) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnUnregisterCallback(hTimer, hCallback);
}
BT_EXPORT_SYMBOL(BT_TimerUnregisterCallback);

BT_u32 BT_TimerGetPrescaler(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnGetPrescaler(hTimer, pError);
}
BT_EXPORT_SYMBOL(BT_TimerGetPrescaler);

BT_ERROR BT_TimerSetPrescaler(BT_HANDLE hTimer, BT_u32 ulPrescaler) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnSetPrescaler(hTimer, ulPrescaler);
}
BT_EXPORT_SYMBOL(BT_TimerSetPrescaler);

BT_u32 BT_TimerGetPeriodCount(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnGetPeriodCount(hTimer, pError);
}
BT_EXPORT_SYMBOL(BT_TimerGetPeriodCount);

BT_ERROR BT_TimerSetPeriodCount(BT_HANDLE hTimer, BT_u32 ulPeriodCount) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnSetPeriodCount(hTimer, ulPeriodCount);
}
BT_EXPORT_SYMBOL(BT_TimerSetPeriodCount);

BT_ERROR BT_TimerSetFrequency(BT_HANDLE hTimer, BT_u32 ulFrequencyHz) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 ulPeriodCount = BT_IF_TIMER_OPS(hTimer)->pfnGetInputClock(hTimer, &Error);
	if (ulFrequencyHz != 0)
	ulPeriodCount /= ulFrequencyHz;

	if (Error != BT_ERR_NONE) return Error;

	return BT_IF_TIMER_OPS(hTimer)->pfnSetPeriodCount(hTimer, ulPeriodCount);
}
BT_EXPORT_SYMBOL(BT_TimerSetFrequency);

BT_u32 BT_TimerGetFrequency(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	BT_u32 ulSysFrequencyHz = BT_IF_TIMER_OPS(hTimer)->pfnGetInputClock(hTimer, pError);

	if (*pError != BT_ERR_NONE) return 0;

	return ulSysFrequencyHz / BT_IF_TIMER_OPS(hTimer)->pfnGetPeriodCount(hTimer, pError);
}
BT_EXPORT_SYMBOL(BT_TimerGetFrequency);

BT_ERROR BT_TimerEnableReload(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnEnableReload(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerEnableReload);

BT_ERROR BT_TimerDisableReload(BT_HANDLE hTimer) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnDisableReload(hTimer);
}
BT_EXPORT_SYMBOL(BT_TimerDisableReload);

BT_u32 BT_TimerGetValue(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnGetValue(hTimer, pError);
}
BT_EXPORT_SYMBOL(BT_TimerGetValue);

BT_ERROR BT_TimerSetValue(BT_HANDLE hTimer, BT_u32 ulValue) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_TIMER_OPS(hTimer)->pfnSetValue(hTimer, ulValue);
}
BT_EXPORT_SYMBOL(BT_TimerSetValue);

BT_BOOL BT_TimerExpired(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return 0;
}
BT_EXPORT_SYMBOL(BT_TimerExpired);

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_TimerSetConfiguration(BT_HANDLE hTimer, void *pConfig) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_TIMER_OPS(hTimer)->pfnSetConfig(hTimer, pConfig);
}
BT_EXPORT_SYMBOL(BT_TimerSetConfiguration);

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_TimerGetConfiguration(BT_HANDLE hTimer, void *pConfig) {
	if(!isTimerHandle(hTimer)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_TIMER_OPS(hTimer)->pfnGetConfig(hTimer, pConfig);
}
BT_EXPORT_SYMBOL(BT_TimerGetConfiguration);
