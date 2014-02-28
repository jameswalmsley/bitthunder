/**
 *	Defines the system control block of cortex mx devices
 *
 *
 *
 **/
#ifndef _SCB_H_
#define _SCB_H_

#include <bitthunder.h>

typedef struct _SCB_REGS {
	BT_u32 ACTLR;
	BT_STRUCT_RESERVED_u32(1, 0x008, 0xD00);
	BT_u32 CPUID;
	BT_u32 ICSR;
	BT_u32 VTOR;
	BT_u32 AIRCR;

#define SCB_AIRCR_VECTKEY			0x05FA0000  // Vector key
#define SCB_AIRCR_SYSRESETREQ		0x00000004  // System reset request

	BT_u32 SCR;
	BT_u32 CCR;
	BT_u8 SHPR[12];
	BT_u32 SHCRS;
	union {
		BT_u32 CFSR;
		BT_u8 MMSR;
		BT_u8 BFSR;
		BT_u16 UFSR;
	};
	BT_u32 HFSR;
	BT_u32 MMFAR;
	BT_u32 BFAR;
} SCB_REGS;

#define SCB_BASE	0xE000E008
#define SCB 		((SCB_REGS *) (SCB_BASE))



#endif
