#ifndef _PWM_H_
#define _PWM_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LPC17xx_PWM_oDeviceInterface;

typedef struct _LPC17xx_PWM_REGS {
	BT_u32 	PWMIR;       // 0x000       Interrupt Register
	BT_u32 	PWMTCR;      // 0x004       PWM Control Register

#define	LPC17xx_PWM_PWMTCR_CEn			0x00000001
#define	LPC17xx_PWM_PWMTCR_CRst		0x00000002
#define	LPC17xx_PWM_PWMTCR_PWMEn		0x00000008

	BT_u32 	PWMTC;       // 0x008       PWM Counter
	BT_u32 	PWMPR;       // 0x00C       Prescale Register
	BT_u32 	PWMPC;       // 0x010       Prescale Counter
	BT_u32 	PWMMCR;      // 0x014       Match Control Register

#define LPC17xx_PWM_PWMMCR_MR0I			0x00000001
#define LPC17xx_PWM_PWMMCR_MR0R			0x00000002
#define LPC17xx_PWM_PWMMCR_MR0S			0x00000004
#define LPC17xx_PWM_PWMMCR_MR1I			0x00000008
#define LPC17xx_PWM_PWMMCR_MR1R			0x00000010
#define LPC17xx_PWM_PWMMCR_MR1S			0x00000020
#define LPC17xx_PWM_PWMMCR_MR2I			0x00000040
#define LPC17xx_PWM_PWMMCR_MR2R			0x00000080
#define LPC17xx_PWM_PWMMCR_MR2S			0x00000100
#define LPC17xx_PWM_PWMMCR_MR3I			0x00000200
#define LPC17xx_PWM_PWMMCR_MR3R			0x00000400
#define LPC17xx_PWM_PWMMCR_MR3S			0x00000800
#define LPC17xx_PWM_PWMMCR_MR4I			0x00001000
#define LPC17xx_PWM_PWMMCR_MR4R			0x00002000
#define LPC17xx_PWM_PWMMCR_MR4S			0x00004000
#define LPC17xx_PWM_PWMMCR_MR5I			0x00008000
#define LPC17xx_PWM_PWMMCR_MR5R			0x00010000
#define LPC17xx_PWM_PWMMCR_MR5S			0x00020000
#define LPC17xx_PWM_PWMMCR_MR6I			0x00040000
#define LPC17xx_PWM_PWMMCR_MR6R			0x00080000
#define LPC17xx_PWM_PWMMCR_MR6S			0x00100000

	BT_u32 	PWMMR0;      // 0x018       Match Register 0
	BT_u32 	PWMMR1;      // 0x01C       Match Register 1
	BT_u32 	PWMMR2;      // 0x020       Match Register 2
	BT_u32 	PWMMR3;      // 0x024       Match Register 3
	BT_u32 	PWMCCR;      // 0x028       Capture Control Register
	BT_u32 	PWMCR0;      // 0x02C       Capture Register 0
	BT_u32 	PWMCR1;      // 0x030       Capture Register 1
	BT_u32 	PWMCR2;      // 0x034       Capture Register 1
	BT_u32 	PWMCR3;      // 0x038       Capture Register 1
	BT_STRUCT_RESERVED_u32(0, 0x38, 0x40);
	BT_u32 	PWMMR4;      // 0x040       Match Register 4
	BT_u32 	PWMMR5;      // 0x044       Match Register 5
	BT_u32 	PWMMR6;      // 0x048       Match Register 6
	BT_u32 	PWMPCR;      // 0x04C       pwm control register

#define	LPC17xx_PWM_PWMPCR_ENA1			0x00000200
#define	LPC17xx_PWM_PWMPCR_ENA2			0x00000400
#define	LPC17xx_PWM_PWMPCR_ENA3			0x00000800
#define	LPC17xx_PWM_PWMPCR_ENA4			0x00001000
#define	LPC17xx_PWM_PWMPCR_ENA5			0x00002000
#define	LPC17xx_PWM_PWMPCR_ENA6			0x00004000

	BT_u32 	PWMLER;      // 0x050       load enable register

#define	LPC17xx_PWM_PWMLER_ENA0			0x00000001
#define	LPC17xx_PWM_PWMLER_ENA1			0x00000002
#define	LPC17xx_PWM_PWMLER_ENA2			0x00000004
#define	LPC17xx_PWM_PWMLER_ENA3			0x00000008
#define	LPC17xx_PWM_PWMLER_ENA4			0x00000010
#define	LPC17xx_PWM_PWMLER_ENA5			0x00000020
#define	LPC17xx_PWM_PWMLER_ENA6			0x00000040

	BT_STRUCT_RESERVED_u32(1, 0x50, 0x70);
	BT_u32 	PWMCTCR;     // 0x070       Count Control Register
} LPC17xx_PWM_REGS;

#define PWM0						((LPC17xx_PWM_REGS *) BT_CONFIG_MACH_LPC17xx_PWM0_BASE)

#endif

