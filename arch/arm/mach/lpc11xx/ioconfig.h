/**
 *	Defines the Reset and Clock Control Register definitions for STM32
 *
 *
 *
 **/
#ifndef _IOCON_H_
#define _IOCON_H_

#include <bitthunder.h>

typedef struct _LPC11xx_IOCON_REGS {
	BT_u32		LPC11xx_IOCON_PIO2_6;					// 0x000 Offset
	BT_STRUCT_RESERVED_u32(0, 0x00, 0x08);				// 0x004 Offset
	BT_u32		LPC11xx_IOCON_PIO2_0;					// 0x008 Offset
	BT_u32		LPC11xx_IOCON_RESET_PIO0_0;				// 0x00C Offset
	BT_u32		LPC11xx_IOCON_PIO0_1;					// 0x010 Offset
	BT_u32		LPC11xx_IOCON_PIO1_8;					// 0x014 Offset
	BT_STRUCT_RESERVED_u32(1, 0x14, 0x1C);				// 0x018 Offset
	BT_u32		LPC11xx_IOCON_PIO0_2;					// 0x01C Offset
	BT_u32		LPC11xx_IOCON_PIO2_7;					// 0x020 Offset
	BT_u32		LPC11xx_IOCON_PIO2_8;					// 0x024 Offset
	BT_u32		LPC11xx_IOCON_PIO2_1;					// 0x028 Offset
	BT_u32		LPC11xx_IOCON_PIO0_3;					// 0x02C Offset
	BT_u32		LPC11xx_IOCON_PIO0_4;					// 0x030 Offset
	BT_u32		LPC11xx_IOCON_PIO0_5;					// 0x034 Offset
	BT_u32		LPC11xx_IOCON_PIO1_9;					// 0x038 Offset
	BT_u32		LPC11xx_IOCON_PIO3_4;					// 0x03C Offset
	BT_u32		LPC11xx_IOCON_PIO2_4;					// 0x040 Offset
	BT_u32		LPC11xx_IOCON_PIO2_5;					// 0x044 Offset
	BT_u32		LPC11xx_IOCON_PIO3_5;					// 0x048 Offset
	BT_u32		LPC11xx_IOCON_PIO0_6;					// 0x04C Offset
	BT_u32		LPC11xx_IOCON_PIO0_7;					// 0x050 Offset
	BT_u32		LPC11xx_IOCON_PIO2_9;					// 0x054 Offset
	BT_u32		LPC11xx_IOCON_PIO2_10;					// 0x058 Offset
	BT_u32		LPC11xx_IOCON_PIO2_2;					// 0x05C Offset
	BT_u32		LPC11xx_IOCON_PIO0_8;					// 0x060 Offset
	BT_u32		LPC11xx_IOCON_SWCLK_PIO0_9;				// 0x064 Offset
	BT_u32		LPC11xx_IOCON_PIO0_10;					// 0x068 Offset
	BT_u32		LPC11xx_IOCON_PIO1_10;					// 0x06C Offset
	BT_u32		LPC11xx_IOCON_PIO2_11;					// 0x070 Offset
	BT_u32		LPC11xx_IOCON_PIO0_11;					// 0x074 Offset
	BT_u32		LPC11xx_IOCON_PIO1_0;					// 0x078 Offset
	BT_u32		LPC11xx_IOCON_PIO1_1;					// 0x07C Offset
	BT_u32		LPC11xx_IOCON_PIO1_2;					// 0x080 Offset
	BT_u32		LPC11xx_IOCON_PIO3_0;					// 0x084 Offset
	BT_u32		LPC11xx_IOCON_PIO3_1;					// 0x088 Offset
	BT_u32		LPC11xx_IOCON_PIO2_3;					// 0x08C Offset
	BT_u32		LPC11xx_IOCON_SWDIO_PIO1_3;				// 0x090 Offset
	BT_u32		LPC11xx_IOCON_PIO1_4;					// 0x094 Offset
	BT_u32		LPC11xx_IOCON_PIO1_11;					// 0x098 Offset
	BT_u32		LPC11xx_IOCON_PIO3_2;					// 0x09C Offset
	BT_u32		LPC11xx_IOCON_PIO1_5;					// 0x0A0 Offset
	BT_u32		LPC11xx_IOCON_PIO1_6;					// 0x0A4 Offset

#define LPC11xx_IOCON_PIO1_6_FUNC_RXD					0x00000001
#define LPC11xx_IOCON_PIO1_6_FUNC_CT32B0_MAT0			0x00000002
#define LPC11xx_IOCON_PIO1_6_MODE_PD					0x00000008
#define LPC11xx_IOCON_PIO1_6_MODE_PU					0x00000010
#define LPC11xx_IOCON_PIO1_6_MODE_REPEATER				0x00000018
#define LPC11xx_IOCON_PIO1_6_HYST_ENABLE				0x00000020
#define LPC11xx_IOCON_PIO1_6_OD							0x00000400


	BT_u32		LPC11xx_IOCON_PIO1_7;					// 0x0A8 Offset

#define LPC11xx_IOCON_PIO1_7_FUNC_TXD					0x00000001
#define LPC11xx_IOCON_PIO1_7_FUNC_CT32B0_MAT1			0x00000002
#define LPC11xx_IOCON_PIO1_7_MODE_PD					0x00000008
#define LPC11xx_IOCON_PIO1_7_MODE_PU					0x00000010
#define LPC11xx_IOCON_PIO1_7_MODE_REPEATER				0x00000018
#define LPC11xx_IOCON_PIO1_7_HYST_ENABLE				0x00000020
#define LPC11xx_IOCON_PIO1_7_OD							0x00000400

	BT_u32		LPC11xx_IOCON_PIO3_3;					// 0x0AC Offset
	BT_u32		LPC11xx_IOCON_SCK_LOC;					// 0x0B0 Offset
	BT_u32		LPC11xx_IOCON_DSR_LOC;					// 0x0B4 Offset
	BT_u32		LPC11xx_IOCON_DCD_LOC;					// 0x0B8 Offset
	BT_u32		LPC11xx_IOCON_RI_LOC;					// 0x0BC Offset
} LPC11xx_IOCON_REGS;

#define LPC11xx_IOCON_BASE	0x40044000
#define LPC11xx_IOCON 		((LPC11xx_IOCON_REGS *) (LPC11xx_IOCON_BASE))


#define	LPC11xx_PIO1_6			&LPC11xx_IOCON->LPC11xx_IOCON_PIO1_6
#define	LPC11xx_PIO1_7			&LPC11xx_IOCON->LPC11xx_IOCON_PIO1_7


void BT_LPC11xx_SetIOConfig(BT_u32 * pIOCON, BT_u32 config);

#endif

