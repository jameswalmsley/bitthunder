/**
 *	MCPWM Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isMCPwmHandle(BT_HANDLE hMCPwm) {
	if(!hMCPwm || !BT_IF_DEVICE(hMCPwm) || (BT_IF_DEVICE_TYPE(hMCPwm) != BT_DEV_IF_T_MCPWM)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_ERROR BT_MCPwmStart(BT_HANDLE hMCPwm, BT_u32 ulChannel) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnStart(hMCPwm, ulChannel);
}

BT_ERROR BT_MCPwmStop(BT_HANDLE hMCPwm, BT_u32 ulChannel) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnStop(hMCPwm, ulChannel);
}

BT_ERROR BT_MCPwmGetChannelConfig(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnGetChannelConfig(hMCPwm, ulChannel, pConfig);
}


BT_ERROR BT_MCPwmSetChannelConfig(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnSetChannelConfig(hMCPwm, ulChannel, pConfig);
}

BT_u32 BT_MCPwmGetChannelPulsewidth(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_ERROR *pError) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnGetChannelPulsewidth(hMCPwm, ulChannel, pError);
}

BT_ERROR BT_MCPwmSetChannelPulsewidth	(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_u32 ulValue) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnSetChannelPulsewidth(hMCPwm, ulChannel, ulValue);
}

BT_ERROR BT_MCPwmSetDCModePattern(BT_HANDLE hMCPwm, BT_MCPWM_DCMODE_PATTERN ulChannel0, BT_MCPWM_DCMODE_PATTERN ulChannel1, BT_MCPWM_DCMODE_PATTERN ulChannel2) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_MCPWM_OPS(hMCPwm)->pfnSetDCModePattern(hMCPwm, ulChannel0, ulChannel1, ulChannel2);

}

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_MCPwmSetConfiguration(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_MCPWM_OPS(hMCPwm)->pfnSetConfig(hMCPwm, pConfig);
}

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_MCPwmGetConfiguration(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig) {
	if(!isMCPwmHandle(hMCPwm)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_MCPWM_OPS(hMCPwm)->pfnGetConfig(hMCPwm, pConfig);
}
