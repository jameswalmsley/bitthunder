/**
 *	BitThunder Platform independent Memory accessors.
 **/

#ifndef _BT_GETMEM_H_
#define _BT_GETMEM_H_

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
#endif

typedef union {
	BT_u16 		u16;
	BT_LE_SHORT	bytes;
} BT_LE_UN16;

typedef union {
	BT_u32		u32;
	BT_LE_LONG	bytes;
} BT_LE_UN32;

BT_u32 BT_GetLongLE		(void *pBuffer, BT_u32 ulOffset);
BT_u16 BT_GetShortLE	(void *pBuffer, BT_u32 ulOffset);

#endif
