#ifndef _BT_IF_CPU_H_
#define _BT_IF_CPU_H_

BT_u32 		BT_GetCoreID			();
BT_u32 		BT_GetTotalCores		();

BT_ERROR 	BT_BootCore				(BT_u32 ulCoreID, void *address, bt_register_t a, bt_register_t b, bt_register_t c, bt_register_t d);

BT_ERROR 	BT_DCacheEnable			();
BT_ERROR 	BT_DCacheDisable		();
BT_ERROR 	BT_DCacheFlush			();
BT_ERROR 	BT_DCacheInvalidate		();
BT_ERROR 	BT_DCacheInvalidateLine(void *addr);
BT_ERROR 	BT_DCacheInvalidateRange(void *addr, BT_u32 len);

BT_ERROR 	BT_ICacheEnable			();
BT_ERROR 	BT_ICacheDisable		();
BT_ERROR 	BT_ICacheInvalidate		();

#endif
