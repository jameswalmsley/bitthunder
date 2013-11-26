#ifndef _BT_DEV_IF_MCPWM_H_
#define _BT_DEV_IF_MCPWM_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef enum {
	BT_MCPWM_MODE_INDEPENDENT=0,
	BT_MCPWM_MODE_DC,
	BT_MCPWM_MODE_AC,
} BT_MCPWM_OPERATING_MODE;

typedef enum {
	BT_MCPWM_DCMODE_PATTERN_NONE = 0,
	BT_MCPWM_DCMODE_PATTERN_BOTTOM,
	BT_MCPWM_DCMODE_PATTERN_TOP,
	BT_MCPWM_DCMODE_PATTERN_BOTH,
} BT_MCPWM_DCMODE_PATTERN;

typedef enum {
	BT_MCPWM_CHANNEL_MODE_EDGE_ALIGNED=0,
	BT_MCPWM_CHANNEL_MODE_CENTER_ALIGNED,
} BT_MCPWM_CHANNEL_TYPE;

typedef enum {
	BT_MCPWM_CHANNEL_POLARITY_PASSIVE_LO=0,
	BT_MCPWM_CHANNEL_POLARITY_PASSIVE_HI,
} BT_MCPWM_CHANNEL_POLARITY;


typedef struct {
	BT_MCPWM_OPERATING_MODE			eMode;
	BT_BOOL							bOutputsInverted;
	BT_u32							ulCommutationPattern;
} BT_MCPWM_CONFIG;

typedef struct {
	BT_MCPWM_CHANNEL_TYPE			eType;
	BT_MCPWM_CHANNEL_POLARITY		ePolarity;
	BT_BOOL							bDeadtimeEnable;
	BT_u32							ulDeadtime;
	BT_BOOL							bUpdateEnable;
	BT_u32							ulTimeValue;
	BT_u32							ulFrequency;
	BT_u32							ulPulsewidth;
} BT_MCPWM_CHANNEL_CONFIG;


typedef struct _BT_DEV_IF_MCPWM {
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig);
	BT_ERROR 	(*pfnStart)					(BT_HANDLE hMCPwm, BT_u32 ulChannel);
	BT_ERROR	(*pfnStop)					(BT_HANDLE hMCPwm, BT_u32 ulChannel);
	BT_ERROR	(*pfnGetChannelConfig)		(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig);
	BT_ERROR 	(*pfnSetChannelConfig)		(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig);
	BT_u32		(*pfnGetChannelPulsewidth)	(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_ERROR *pError);
	BT_ERROR 	(*pfnSetChannelPulsewidth)	(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_u32 ulValue);
	BT_ERROR	(*pfnSetDCModePattern)		(BT_HANDLE hMCPwm, BT_MCPWM_DCMODE_PATTERN ulChannel0, BT_MCPWM_DCMODE_PATTERN ulChannel1, BT_MCPWM_DCMODE_PATTERN ulChannel2);
} BT_DEV_IF_MCPWM;

BT_ERROR 	BT_MCPwmStart					(BT_HANDLE hMCPwm, BT_u32 ulChannel);
BT_ERROR 	BT_MCPwmStop					(BT_HANDLE hMCPwm, BT_u32 ulChannel);

BT_ERROR 	BT_MCPwmSetConfiguration		(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig);
BT_ERROR 	BT_MCPwmGetConfiguration		(BT_HANDLE hMCPwm, BT_MCPWM_CONFIG *pConfig);

BT_ERROR	BT_MCPwmGetChannelConfig		(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig);
BT_ERROR	BT_MCPwmSetChannelConfig		(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig);

BT_u32		BT_MCPwmGetChannelPulsewidth	(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_ERROR *pError);
BT_ERROR	BT_MCPwmSetChannelPulsewidth	(BT_HANDLE hMCPwm, BT_u32 ulChannel, BT_u32 ulValue);

BT_ERROR	BT_MCPwmSetDCModePattern		(BT_HANDLE hMCPwm, BT_MCPWM_DCMODE_PATTERN ulChannel0, BT_MCPWM_DCMODE_PATTERN ulChannel1, BT_MCPWM_DCMODE_PATTERN ulChannel2);

#endif
