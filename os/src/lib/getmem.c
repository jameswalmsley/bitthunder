/**
 *	BitThunder Platform independent Memory Accessors.
 **/

#include <bt_types.h>
#include <bt_config.h>
#include <lib/getmem.h>
#include <bt_module.h>

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
BT_EXPORT_SYMBOL(bt_cpu_to_le64);

BT_le32 bt_cpu_to_le32(BT_u32 u32) {

	BT_LE_UN32 un32;
	BT_u8 *p = (BT_u8 *) &u32;

	un32.bytes.u8_3 = p[3];
	un32.bytes.u8_2 = p[2];
	un32.bytes.u8_1 = p[1];
	un32.bytes.u8_0 = p[0];

	return un32.u32;
}
BT_EXPORT_SYMBOL(bt_cpu_to_le32);

BT_le16 bt_cpu_to_le16(BT_u16 u16) {

	BT_LE_UN16 un16;
	BT_u8 *p = (BT_u8 *) &u16;

	un16.bytes.u8_1 = p[1];
	un16.bytes.u8_0 = p[0];

	return un16.u16;
}
BT_EXPORT_SYMBOL(bt_cpu_to_le16);
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
BT_EXPORT_SYMBOL(bt_cpu_to_be64);

BT_be32 bt_cpu_to_be32(BT_u32 u32) {

	BT_BE_UN32 un32;
	BT_u8 *p = (BT_u8 *) &u32;

	un32.bytes.u8_3 = p[3];
	un32.bytes.u8_2 = p[2];
	un32.bytes.u8_1 = p[1];
	un32.bytes.u8_0 = p[0];

	return un32.u32;
}
BT_EXPORT_SYMBOL(bt_cpu_to_be32);

BT_be16 bt_cpu_to_be16(BT_u16 u16) {

	BT_BE_UN16 un16;
	BT_u8 *p = (BT_u8 *) &u16;

	un16.bytes.u8_1 = p[1];
	un16.bytes.u8_0 = p[0];

	return un16.u16;
}
BT_EXPORT_SYMBOL(bt_cpu_to_be16);

BT_u64 bt_be64_to_cpu(BT_be64 be64) {
	return __builtin_bswap64(be64);
}
BT_EXPORT_SYMBOL(bt_be64_to_cpu);

BT_u32 bt_be32_to_cpu(BT_be32 be32) {
	return __builtin_bswap32(be32);
}
BT_EXPORT_SYMBOL(bt_be32_to_cpu);

BT_u16 bt_be16_to_cpu(BT_be16 be16) {
	return __builtin_bswap16(be16);
}
BT_EXPORT_SYMBOL(bt_be16_to_cpu);
#endif

BT_u64 BT_Get64LE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = (BT_u8 *) (pBuffer) + ulOffset;
	BT_LE_UN64 u64;

	u64.bytes.u8_7 = p[7];
	u64.bytes.u8_6 = p[6];
	u64.bytes.u8_5 = p[5];
	u64.bytes.u8_4 = p[4];

	u64.bytes.u8_3 = p[3];
	u64.bytes.u8_2 = p[2];
	u64.bytes.u8_1 = p[1];
	u64.bytes.u8_0 = p[0];

	return u64.u64;
}
BT_EXPORT_SYMBOL(BT_Get64LE);

BT_u32 BT_Get32LE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_LE_UN32 u32;

	u32.bytes.u8_3 = p[3];
	u32.bytes.u8_2 = p[2];
	u32.bytes.u8_1 = p[1];
	u32.bytes.u8_0 = p[0];

	return u32.u32;
}
BT_EXPORT_SYMBOL(BT_Get32LE);

BT_u16 BT_Get16LE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_LE_UN16 u16;

	u16.bytes.u8_1 = p[1];
	u16.bytes.u8_0 = p[0];

	return u16.u16;
}
BT_EXPORT_SYMBOL(BT_Get16LE);

BT_u64 BT_Get64BE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = (BT_u8 *) (pBuffer) + ulOffset;
	BT_BE_UN64 u64;

	u64.bytes.u8_7 = p[7];
	u64.bytes.u8_6 = p[6];
	u64.bytes.u8_5 = p[5];
	u64.bytes.u8_4 = p[4];

	u64.bytes.u8_3 = p[3];
	u64.bytes.u8_2 = p[2];
	u64.bytes.u8_1 = p[1];
	u64.bytes.u8_0 = p[0];

	return u64.u64;
}
BT_EXPORT_SYMBOL(BT_Get64BE);

BT_u32 BT_Get32BE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_BE_UN32 u32;

	u32.bytes.u8_3 = p[3];
	u32.bytes.u8_2 = p[2];
	u32.bytes.u8_1 = p[1];
	u32.bytes.u8_0 = p[0];

	return u32.u32;
}
BT_EXPORT_SYMBOL(BT_Get32BE);

BT_u16 BT_Get16BE(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_BE_UN16 u16;

	u16.bytes.u8_1 = p[1];
	u16.bytes.u8_0 = p[0];

	return u16.u16;
}
BT_EXPORT_SYMBOL(BT_Get16BE);

BT_u8 BT_Get8(void *pBuffer, BT_u32 ulOffset) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	return *p;
}

void BT_Put64LE(void *pBuffer, BT_u32 ulOffset, BT_u64 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_LE_UN64 u64;

	u64.u64 = ulValue;
	p[0] = u64.bytes.u8_0;
	p[1] = u64.bytes.u8_1;
	p[2] = u64.bytes.u8_2;
	p[3] = u64.bytes.u8_3;

	p[4] = u64.bytes.u8_4;
	p[5] = u64.bytes.u8_5;
	p[6] = u64.bytes.u8_6;
	p[7] = u64.bytes.u8_7;
}
BT_EXPORT_SYMBOL(BT_Put64LE);

void BT_Put32LE(void *pBuffer, BT_u32 ulOffset, BT_u32 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_LE_UN32 u32;

	u32.u32 = ulValue;
	p[0] = u32.bytes.u8_0;
	p[1] = u32.bytes.u8_1;
	p[2] = u32.bytes.u8_2;
	p[3] = u32.bytes.u8_3;
}
BT_EXPORT_SYMBOL(BT_Put32LE);

void BT_Put16LE(void *pBuffer, BT_u32 ulOffset, BT_u16 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_LE_UN16 u16;

	u16.u16 = ulValue;

	p[0] = u16.bytes.u8_0;
	p[1] = u16.bytes.u8_1;
}
BT_EXPORT_SYMBOL(BT_Put16LE);

void BT_Put64BE(void *pBuffer, BT_u32 ulOffset, BT_u64 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_BE_UN64 u64;

	u64.u64 = ulValue;
	p[0] = u64.bytes.u8_0;
	p[1] = u64.bytes.u8_1;
	p[2] = u64.bytes.u8_2;
	p[3] = u64.bytes.u8_3;

	p[4] = u64.bytes.u8_4;
	p[5] = u64.bytes.u8_5;
	p[6] = u64.bytes.u8_6;
	p[7] = u64.bytes.u8_7;
}
BT_EXPORT_SYMBOL(BT_Put64BE);

void BT_Put32BE(void *pBuffer, BT_u32 ulOffset, BT_u32 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_BE_UN32 u32;

	u32.u32 = ulValue;
	p[0] = u32.bytes.u8_0;
	p[1] = u32.bytes.u8_1;
	p[2] = u32.bytes.u8_2;
	p[3] = u32.bytes.u8_3;
}
BT_EXPORT_SYMBOL(BT_Put32BE);

void BT_Put16BE(void *pBuffer, BT_u32 ulOffset, BT_u16 ulValue) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	BT_BE_UN16 u16;

	u16.u16 = ulValue;

	p[0] = u16.bytes.u8_0;
	p[1] = u16.bytes.u8_1;
}
BT_EXPORT_SYMBOL(BT_Put16BE);

void BT_Put8(void *pBuffer, BT_u32 ulOffset, BT_u8 value) {
	BT_u8 *p = ((BT_u8 *) pBuffer) + ulOffset;
	*p = value;
}
BT_EXPORT_SYMBOL(BT_Put8);
