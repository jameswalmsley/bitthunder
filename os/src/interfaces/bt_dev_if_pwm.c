/**
 *	PWM Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_BOOL isPwmHandle(BT_HANDLE hPwm) {
	if(!hPwm || !BT_IF_DEVICE(hPwm) || (BT_IF_DEVICE_TYPE(hPwm) != BT_DEV_IF_T_PWM)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

BT_ERROR BT_PwmStart(BT_HANDLE hPwm) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnStart(hPwm);
}
BT_EXPORT_SYMBOL(BT_PwmStart);

BT_ERROR BT_PwmStop(BT_HANDLE hPwm) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnStop(hPwm);
}
BT_EXPORT_SYMBOL(BT_PwmStop);

BT_u32 BT_PwmGetPeriodCount(BT_HANDLE hPwm, BT_ERROR *pError) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnGetPeriodCount(hPwm, pError);
}
BT_EXPORT_SYMBOL(BT_PwmGetPeriodCount);

BT_ERROR BT_PwmSetFrequency(BT_HANDLE hPwm, BT_u32 ulFrequencyHz) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnSetFrequency(hPwm, ulFrequencyHz);
}
BT_EXPORT_SYMBOL(BT_PwmSetFrequency);

BT_u32 BT_PwmGetFrequency(BT_HANDLE hPwm, BT_ERROR *pError) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnGetFrequency(hPwm, pError);

}
BT_EXPORT_SYMBOL(BT_PwmGetFrequency);

BT_u32 BT_PwmGetDutyCycle(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnGetDutyCycle(hPwm, ulChannel, pError);
}
BT_EXPORT_SYMBOL(BT_PwmGetDutyCycle);

BT_ERROR BT_PwmSetDutyCycle(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnSetDutyCycle(hPwm, ulChannel, ulValue);
}
BT_EXPORT_SYMBOL(BT_PwmSetDutyCycle);

BT_u32 BT_PwmGetDeadTime(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnGetDeadTime(hPwm, ulChannel, pError);
}
BT_EXPORT_SYMBOL(BT_PwmGetDeadTime);

BT_ERROR BT_PwmSetDeadTime(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_PWM_OPS(hPwm)->pfnSetDeadTime(hPwm, ulChannel, ulValue);
}
BT_EXPORT_SYMBOL(BT_PwmSetDeadTime);

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_PwmSetConfiguration(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_PWM_OPS(hPwm)->pfnSetConfig(hPwm, pConfig);
}
BT_EXPORT_SYMBOL(BT_PwmSetConfiguration);

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_PwmGetConfiguration(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig) {
	if(!isPwmHandle(hPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_PWM_OPS(hPwm)->pfnGetConfig(hPwm, pConfig);
}
BT_EXPORT_SYMBOL(BT_PwmGetConfiguration);
