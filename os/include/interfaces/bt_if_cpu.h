#ifndef _BT_IF_CPU_H_
#define _BT_IF_CPU_H_

BT_u32 		BT_GetCoreID			();
BT_u32 		BT_GetTotalCores		();

BT_ERROR 	BT_SetCoreStartAddress	(BT_u32 ulCoreID, void *pStart);
BT_ERROR 	BT_StartCore			(BT_u32 ulCoreID);

BT_ERROR 	BT_DCacheEnable			();
BT_ERROR 	BT_DCacheDisable		();
BT_ERROR 	BT_DCacheFlush			();
BT_ERROR 	BT_DCacheInvalidate		();

BT_ERROR 	BT_ICacheEnable			();
BT_ERROR 	BT_ICacheDisable		();
BT_ERROR 	BT_ICacheInvalidate		();

#endif
