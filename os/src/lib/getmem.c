/**
 *	BitThunder Platform independent Memory Accessors.
 **/

#include <bt_types.h>
#include <bt_config.h>
#include <lib/getmem.h>

BT_u32 BT_GetLongLE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = (BT_u8 *) pBuffer;
	BT_LE_UN32 u32;
	p += ulOffset;

	u32.bytes.u8_3 = p[3];
	u32.bytes.u8_2 = p[2];
	u32.bytes.u8_1 = p[1];
	u32.bytes.u8_0 = p[0];

	return u32.u32;
}

BT_u16 BT_GetShortLE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = (BT_u8 *) pBuffer;
	BT_LE_UN16 u16;
	p += ulOffset;

	u16.bytes.u8_1 = p[1];
	u16.bytes.u8_0 = p[0];

	return u16.u16;
}
