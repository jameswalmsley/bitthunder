/**
 *	BitThunder Platform independent Memory Accessors.
 **/

#include <bt_types.h>
#include <bt_config.h>
#include <lib/getmem.h>

#ifdef BT_CONFIG_BIG_ENDIAN
BT_le64 bt_cpu_to_le64(BT_u64 u64) {

	BT_LE_UN64 un64;
	BT_u8 *p = (BT_u8 *) &u64;

	un64.bytes.u8_7 = p[7];
	un64.bytes.u8_6 = p[6];
	un64.bytes.u8_5 = p[5];
	un64.bytes.u8_4 = p[4];
	un64.bytes.u8_3 = p[3];
	un64.bytes.u8_2 = p[2];
	un64.bytes.u8_1 = p[1];
	un64.bytes.u8_0 = p[0];

	return un64.u64;
}

BT_le32 bt_cpu_to_le32(BT_u32 u32) {

	BT_LE_UN32 un32;
	BT_u8 *p = (BT_u8 *) &u32;

	un32.bytes.u8_3 = p[3];
	un32.bytes.u8_2 = p[2];
	un32.bytes.u8_1 = p[1];
	un32.bytes.u8_0 = p[0];

	return un32.u32;
}

BT_le16 bt_cpu_to_le16(BT_u16 u16) {

	BT_LE_UN16 un16;
	BT_u8 *p = (BT_u8 *) &u16;

	un16.bytes.u8_1 = p[1];
	un16.bytes.u8_0 = p[0];

	return un16.u16;
}
#endif

#ifdef BT_CONFIG_LITTLE_ENDIAN
BT_be64 bt_cpu_to_be64(BT_u64 u64) {

	BT_BE_UN64 un64;
	BT_u8 *p = (BT_u8 *) &u64;

	un64.bytes.u8_7 = p[7];
	un64.bytes.u8_6 = p[6];
	un64.bytes.u8_5 = p[5];
	un64.bytes.u8_4 = p[4];
	un64.bytes.u8_3 = p[3];
	un64.bytes.u8_2 = p[2];
	un64.bytes.u8_1 = p[1];
	un64.bytes.u8_0 = p[0];

	return un64.u64;
}

BT_be32 bt_cpu_to_be32(BT_u32 u32) {

	BT_BE_UN32 un32;
	BT_u8 *p = (BT_u8 *) &u32;

	un32.bytes.u8_3 = p[3];
	un32.bytes.u8_2 = p[2];
	un32.bytes.u8_1 = p[1];
	un32.bytes.u8_0 = p[0];

	return un32.u32;
}

BT_be16 bt_cpu_to_be16(BT_u16 u16) {

	BT_BE_UN16 un16;
	BT_u8 *p = (BT_u8 *) &u16;

	un16.bytes.u8_1 = p[1];
	un16.bytes.u8_0 = p[0];

	return un16.u16;
}

BT_u64 bt_be64_to_cpu(BT_be64 be64) {
	return __builtin_bswap64(be64);
}

BT_u32 bt_be32_to_cpu(BT_be32 be32) {
	return __builtin_bswap32(be32);
}

BT_u16 bt_be16_to_cpu(BT_be16 be16) {
	return __builtin_bswap16(be16);
}
#endif


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
