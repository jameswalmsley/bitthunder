#ifndef _TIMER_H_
#define _TIMER_H_

#include <bitthunder.h>
#include <bt_struct.h>


typedef struct _BT_LPC11xx_TIMER_CONFIG{
	BT_u32	Control;
	BT_u32	MatchControl;
	BT_u32	Match[3];
	BT_u32	CaptureControl;
	BT_u32	Capture;
	BT_u32	ExtMatchControl;
	BT_u32	PWMControl;
} BT_LPC11xx_TIMER_CONFIG;

extern const BT_IF_DEVICE BT_LPC11xx_TIMER_oDeviceInterface;

typedef struct _LPC11xx_TIMER_REGS {
	BT_u32 	TMRBIR;       // 0x000       Interrupt Register
	BT_u32 	TMRBTCR;      // 0x004       Timer Control Register

#define	LPC11xx_TIMER_TMRBTCR_CEn			0x00000001

	BT_u32 	TMRBTC;       // 0x008       Timer Counter
	BT_u32 	TMRBPR;       // 0x00C       Prescale Register
	BT_u32 	TMRBPC;       // 0x010       Prescale Counter
	BT_u32 	TMRBMCR;      // 0x014       Match Control Register

#define LPC11xx_TIMER_TMRMCR_MR0I			0x00000001
#define LPC11xx_TIMER_TMRMCR_MR0R			0x00000002
#define LPC11xx_TIMER_TMRMCR_MR0S			0x00000004
#define LPC11xx_TIMER_TMRMCR_MR1I			0x00000008
#define LPC11xx_TIMER_TMRMCR_MR1R			0x00000010
#define LPC11xx_TIMER_TMRMCR_MR1S			0x00000020
#define LPC11xx_TIMER_TMRMCR_MR2I			0x00000040
#define LPC11xx_TIMER_TMRMCR_MR2R			0x00000080
#define LPC11xx_TIMER_TMRMCR_MR2S			0x00000100
#define LPC11xx_TIMER_TMRMCR_MR3I			0x00000200
#define LPC11xx_TIMER_TMRMCR_MR3R			0x00000400
#define LPC11xx_TIMER_TMRMCR_MR3S			0x00000800

	BT_u32 	TMRBMR0;      // 0x018       Match Register 0
	BT_u32 	TMRBMR1;      // 0x01C       Match Register 1
	BT_u32 	TMRBMR2;      // 0x020       Match Register 2
	BT_u32 	TMRBMR3;      // 0x024       Match Register 3
	BT_u32 	TMRBCCR;      // 0x028       Capture Control Register
	BT_u32 	TMRBCR0;      // 0x02C       Capture Register 0
	BT_STRUCT_RESERVED_u32(0, 0x2C, 0x3C);
	BT_u32 	TMRBEMR;      // 0x03C       External Match Register
	BT_STRUCT_RESERVED_u32(1, 0x3C, 0x70);
	BT_u32 	TMRBCTCR;     // 0x070       Count Control Register
	BT_u32 	TMRBPWMC;     // 0x074       PWM Control Register
	BT_STRUCT_RESERVED_u32(2, 0x74, 0x4000);
} LPC11xx_TIMER_REGS;



#define TIMER0						((LPC11xx_TIMER_REGS *) BT_CONFIG_MACH_LPC11xx_TIMER0_BASE)
#define TIMER1						((LPC11xx_TIMER_REGS *) BT_CONFIG_MACH_LPC11xx_TIMER1_BASE)
#define TIMER2						((LPC11xx_TIMER_REGS *) BT_CONFIG_MACH_LPC11xx_TIMER2_BASE)
#define TIMER3						((LPC11xx_TIMER_REGS *) BT_CONFIG_MACH_LPC11xx_TIMER3_BASE)


#endif

