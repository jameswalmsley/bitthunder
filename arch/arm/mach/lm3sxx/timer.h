#ifndef _TIMER_H_
#define _TIMER_H_

#include <bitthunder.h>
#include <bt_struct.h>


typedef struct _LM3Sxx_TIMER_REGS {
	BT_u32 	TMRCFG;       	// 0x000
	BT_u32 	TMRTAMR;      	// 0x004
	BT_u32 	TMRTBMR;       	// 0x008
	BT_u32 	TMRCTL;      	// 0x00C
	BT_STRUCT_RESERVED_u32(0, 0x0C, 0x18);
	BT_u32 	TMRIMR;       	// 0x018
	BT_u32 	TMRRIS;      	// 0x01C
	BT_u32 	TMRMIS;      	// 0x020
	BT_u32 	TMRICR;      	// 0x024
	BT_u32 	TMRTAILR;      	// 0x028
	BT_u32 	TMRTBILR;		// 0x02C
	BT_u32 	TMRTAMATCHR;    // 0x030
	BT_u32 	TMRTBMATCHR;    // 0x034
	BT_u32 	TMRTAPR;      	// 0x038
	BT_u32 	TMRTBPR;      	// 0x03C
	BT_u32 	TMRTAPMR;   	// 0x040
	BT_u32 	TMRTBPMR; 	   	// 0x044
	BT_u32 	TMRTAR;      	// 0x048
	BT_u32 	TMRTBR;      	// 0x04C
	BT_u32 	TMRTAV;   		// 0x050
	BT_u32 	TMRTBV; 	   	// 0x054
} LM3Sxx_TIMER_REGS;



#define TIMER0						((LM3Sxx_TIMER_REGS *) BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE)
#define TIMER1						((LM3Sxx_TIMER_REGS *) BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE)
#define TIMER2						((LM3Sxx_TIMER_REGS *) BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE)
#define TIMER3						((LM3Sxx_TIMER_REGS *) BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE)


#endif

