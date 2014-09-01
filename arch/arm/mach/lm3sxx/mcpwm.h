#ifndef _MCPWM_H_
#define _MCPWM_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LM3Sxx_MCPWM_oDeviceInterface;

typedef struct _BT_LM3Sxx_MCPWM_BLOCK {
	BT_u32	CTL;			//0x0000
	BT_u32	INTEN;			//0x0004
	BT_u32	RIS;			//0x0008
	BT_u32	ISC;			//0x000C
	BT_u32	LOAD;			//0x0010
	BT_u32	COUNT;			//0x0014
	BT_u32	CMPA;			//0x0018
	BT_u32	CMPB;			//0x001C
	BT_u32	GENA;			//0x0020
	BT_u32	GENB;			//0x0024
	BT_u32	DBCTL;			//0x0028
	BT_u32	DBRISE;			//0x002C
	BT_u32	DBFALL;			//0x0030
	BT_u32	FLTSRC0;		//0x0034
	BT_u32	FLTSRC1;		//0x0038
	BT_u32	MINFLTPER;		//0x003C
} BT_LM3Sxx_MCPWM_BLOCK;

typedef struct _LM3Sxx_MCPWM_REGS {
	BT_u32 	PWMCTL;			// 0x000       Interrupt Register
	BT_u32 	PWMSYNC;		// 0x004       PWM Control Register
	BT_u32 	PWMENABLE;		// 0x008       PWM Counter
	BT_u32 	PWMINVERT;		// 0x00C       Prescale Register
	BT_u32 	PWMFAULT;		// 0x010       Prescale Counter
	BT_u32 	PWMINTEN;		// 0x014       Match Control Register
	BT_u32 	PWMRIS;      // 0x018       Match Register 0
	BT_u32 	PWMISC;      // 0x01C       Match Register 1
	BT_u32 	PWMSTATUS;      // 0x020       Match Register 2
	BT_u32 	PWMFAULTVAL;      // 0x024       Match Register 3
	BT_u32 	PWMENUPD;      // 0x028       Capture Control Register
	BT_STRUCT_RESERVED_u32(0, 0x28, 0x40);
	BT_LM3Sxx_MCPWM_BLOCK	PWMBlocks[4];
	BT_STRUCT_RESERVED_u32(1, 0x13C, 0x800);
	BT_u32	PWM0FLTSEN;
	BT_u32	PWM0FLTSTAT0;
	BT_u32	PWM0FLTSTAT1;
	BT_STRUCT_RESERVED_u32(2, 0x808, 0x880);
	BT_u32	PWM1FLTSEN;
	BT_u32	PWM1FLTSTAT0;
	BT_u32	PWM1FLTSTAT1;
	BT_STRUCT_RESERVED_u32(3, 0x888, 0x900);
	BT_u32	PWM2FLTSEN;
	BT_u32	PWM2FLTSTAT0;
	BT_u32	PWM2FLTSTAT1;
	BT_STRUCT_RESERVED_u32(4, 0x908, 0x980);
	BT_u32	PWM3FLTSEN;
	BT_u32	PWM3FLTSTAT0;
	BT_u32	PWM3FLTSTAT1;
} LM3Sxx_MCPWM_REGS;

#define PWM0						((LM3Sxx_MCPWM_REGS *) BT_CONFIG_MACH_LM3Sxx_MCPWM0_BASE)

#endif

