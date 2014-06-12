#ifndef _SDCARD_H_
#define _SDCARD_H_





#define SDCARD_FLAGS_ALWAYS_PRESENT				0x00000001		///< Flag to forcibly assume card presence. (E.g. when card detection line is not connected).


typedef enum {
	BT_SDCARD_RESPONSE_TYPE_NONE = 0,
	BT_SDCARD_RESPONSE_TYPE_R1,
	BT_SDCARD_RESPONSE_TYPE_R1b,
	BT_SDCARD_RESPONSE_TYPE_R3,
	BT_SDCARD_RESPONSE_TYPE_R4,
	BT_SDCARD_RESPONSE_TYPE_R5,
	BT_SDCARD_RESPONSE_TYPE_R6,
	BT_SDCARD_RESPONSE_TYPE_R7,
	BT_SDCARD_RESPONSE_TYPE_R2,
	BT_SDCARD_RESPONSE_TYPE_R1_DATA,
} BT_SDCARD_RESPONSE_TYPE;


typedef struct
{
	unsigned :1;
	unsigned CRC:7;
	unsigned Month:4;
	unsigned Year:8;
	unsigned :4;
	BT_u32 PSN;
	unsigned PRVMinor:4;
	unsigned PRVMajor:4;
	BT_s8 PNM[5];
	BT_s8 OID[2];
	BT_u8 MID;
} __attribute__((packed)) tCID;

typedef struct
{
	unsigned :1;
	unsigned CRC:7;
	unsigned :2;
	unsigned File_Format:2;
	unsigned Tmp_Write_Protection:1;
	unsigned Perm_Write_Protection:1;
	unsigned Copy:1;
	unsigned File_Format_GRP:1;
	unsigned :5;
	unsigned Write_BL_Partial:1;
	unsigned Write_BL_Len:4;
	unsigned R2W_Factor:3;
	unsigned :2;
	unsigned WP_GRP_Enable:1;
	unsigned WP_GRP_Size:7;
	unsigned Sector_Size:7;
	unsigned Erase_BLK_En:1;
	unsigned C_Size_Mult:3;
	unsigned VDD_W_Curr_Max:3;
	unsigned VDD_W_Curr_Min:3;
	unsigned VDD_R_Curr_Max:3;
	unsigned VDD_R_Curr_Min:3;
	unsigned C_Size:12;
	unsigned :2;
	unsigned DSR_Imp:1;
	unsigned Read_BLK_MisAlign:1;
	unsigned Write_BLK_MisAlign:1;
	unsigned Read_BL_Partial:1;
	unsigned Read_BL_Len:4;
	unsigned CCC:12;
	BT_u8 TRAN_SPEED;
	BT_u8 NSAC;
	BT_u8 TAAC;
	unsigned :6;
	unsigned CSD:2;
} __attribute__((packed))  tCSD1_x;

typedef struct
{
	unsigned :1;
	unsigned CRC:7;
	unsigned :2;
	unsigned File_Format:2;
	unsigned Tmp_Write_Protection:1;
	unsigned Perm_Write_Protection:1;
	unsigned Copy:1;
	unsigned File_Format_GRP:1;
	unsigned :5;
	unsigned Write_BL_Partial:1;
	unsigned Write_BL_Len:4;
	unsigned R2W_Factor:3;
	unsigned :2;
	unsigned WP_GRP_Enable:1;
	unsigned WP_GRP_Size:7;
	unsigned Sector_Size:7;
	unsigned Erase_BLK_En:1;
	unsigned :1;
	unsigned C_Size:22;
	unsigned :6;
	unsigned DSR_Imp:1;
	unsigned Read_BLK_MisAlign:1;
	unsigned Write_BLK_MisAlign:1;
	unsigned Read_BL_Partial:1;
	unsigned Read_BL_Len:4;
	unsigned CCC:12;
	BT_u8 TRAN_SPEED;
	BT_u8 NSAC;
	BT_u8 TAAC;
	unsigned :6;
	unsigned CSD:2;
} __attribute__((packed)) tCSD2_x;







#endif
