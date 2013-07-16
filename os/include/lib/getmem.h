/**
 *	BitThunder Platform independent Memory accessors.
 **/

#ifndef _BT_GETMEM_H_
#define _BT_GETMEM_H_

typedef BT_u64	BT_be64;
typedef BT_u32	BT_be32;
typedef BT_u16	BT_be16;

typedef BT_u64	BT_le64;
typedef BT_u32	BT_le32;
typedef BT_u16	BT_le16;

#if defined (BT_CONFIG_LITTLE_ENDIAN)
typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
} BT_LE_SHORT;

typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
	BT_u8	u8_2;
	BT_u8 	u8_3;
} BT_LE_LONG;

typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
	BT_u8	u8_2;
	BT_u8 	u8_3;
	BT_u8	u8_4;
	BT_u8	u8_5;
	BT_u8	u8_6;
	BT_u8 	u8_7;
} BT_LE_LONGLONG;

typedef struct {
	BT_u8	u8_1;
	BT_u8	u8_0;
} BT_BE_SHORT;

typedef struct {
	BT_u8	u8_3;
	BT_u8	u8_2;
	BT_u8	u8_1;
	BT_u8 	u8_0;
} BT_BE_LONG;

typedef struct {
	BT_u8	u8_7;
	BT_u8	u8_6;
	BT_u8	u8_5;
	BT_u8 	u8_4;
	BT_u8	u8_3;
	BT_u8	u8_2;
	BT_u8	u8_1;
	BT_u8 	u8_0;
} BT_BE_LONGLONG;
#endif

#if defined (BT_CONFIG_BIG_ENDIAN)
typedef struct {
	BT_u8	u8_1;
	BT_u8	u8_0;
} BT_LE_SHORT;

typedef struct {
	BT_u8	u8_3;
	BT_u8	u8_2;
	BT_u8	u8_1;
	BT_u8 	u8_0;
} BT_LE_LONG;

typedef struct {
	BT_u8	u8_7;
	BT_u8	u8_6;
	BT_u8	u8_5;
	BT_u8 	u8_4;
	BT_u8	u8_3;
	BT_u8	u8_2;
	BT_u8	u8_1;
	BT_u8 	u8_0;
} BT_LE_LONGLONG;

typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
} BT_BE_SHORT;

typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
	BT_u8	u8_2;
	BT_u8 	u8_3;
} BT_BE_LONG;

typedef struct {
	BT_u8	u8_0;
	BT_u8	u8_1;
	BT_u8	u8_2;
	BT_u8 	u8_3;
	BT_u8	u8_4;
	BT_u8	u8_5;
	BT_u8	u8_6;
	BT_u8 	u8_7;
} BT_BE_LONGLONG;
#endif

typedef union {
	BT_le16 		u16;
	BT_LE_SHORT		bytes;
} BT_LE_UN16;

typedef union {
	BT_le32			u32;
	BT_LE_LONG		bytes;
} BT_LE_UN32;

typedef union {
	BT_le64			u64;
	BT_LE_LONGLONG	bytes;
} BT_LE_UN64;

typedef union {
	BT_be16 		u16;
	BT_BE_SHORT		bytes;
} BT_BE_UN16;

typedef union {
	BT_be32			u32;
	BT_BE_LONG		bytes;
} BT_BE_UN32;

typedef union {
	BT_be64			u64;
	BT_BE_LONGLONG	bytes;
} BT_BE_UN64;

#ifdef BT_CONFIG_BIG_ENDIAN
#define bt_cpu_to_be64(u64)	(u64)
#define bt_cpu_to_be32(u32)	(u32)
#define bt_cpu_to_be16(u16)	(u16)

#define bt_be64_to_cpu(__be64)	(__be64)
#define bt_be32_to_cpu(__be32)	(__be32)
#define bt_be16_to_cpu(__be16)	(__be16)

BT_le64 bt_cpu_to_le64(BT_u64);
BT_le32 bt_cpu_to_le32(BT_u32);
BT_le16 bt_cpu_to_le16(BT_u16);

BT_u64 bt_le64_to_cpu(BT_le64 u64);
BT_u32 bt_le32_to_cpu(BT_le32 u32);
BT_u32 bt_le16_to_cpu(BT_le16 u16);
#endif

#ifdef BT_CONFIG_LITTLE_ENDIAN
BT_be64 bt_cpu_to_be64(BT_u64 u64);
BT_be32 bt_cpu_to_be32(BT_u32 u32);
BT_be16 bt_cpu_to_be16(BT_u16 u16);

BT_u64 	bt_be64_to_cpu(BT_be64 u64);
BT_u32	bt_be32_to_cpu(BT_be32 u32);
BT_u16	bt_be16_to_cpu(BT_be16 u16);

#define bt_cpu_to_le64(u64)	(u64)
#define bt_cpu_to_le32(u32)	(u32)
#define bt_cpu_to_le16(u16)	(u16)

#define bt_le64_to_cpu(__le64)	(__le64)
#define bt_le32_to_cpu(__le32)	(__le32)
#define bt_le16_to_cpu(__le16)	(__le16)
#endif

BT_u32 BT_GetLongLE		(void *pBuffer, BT_u32 ulOffset);
BT_u16 BT_GetShortLE	(void *pBuffer, BT_u32 ulOffset);

#endif
