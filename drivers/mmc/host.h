#ifndef _HOST_H_
#define _HOST_H_


typedef struct _BT_MMC_HOST_OPS {
	BT_u32	(*pfnGetInputClock)(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError);
} BT_MMC_HOST_OPS;










#endif
