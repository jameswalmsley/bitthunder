#ifndef _BT_DEV_IF_SDIO_H_
#define _BT_DEV_IF_SDIO_H_


typedef struct _BT_DEV_IF_SDIO {
	BT_ERROR (*pfnSendCommand)	(BT_u8 ucCommand, BT_u32 ulArgument, BT_u32 *pResponse);

} BT_DEV_IF_SDIO;

#endif
