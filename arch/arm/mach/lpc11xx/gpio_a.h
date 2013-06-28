#ifndef _GPIO_A_H_
#define _GPIO_A_H_

#include <bitthunder.h>

typedef struct _LPC11Axx_GPIO_REGS {
	BT_u8  BYTE[42];
	BT_u8  padding[2];
	BT_STRUCT_RESERVED_u32(0, 0x0028, 0x1000);
	BT_u32 WORD[42];
	BT_STRUCT_RESERVED_u32(1, 0x10A4, 0x2000);
	BT_u32 DIR[2];		///< Direction register.
	BT_STRUCT_RESERVED_u32(2, 0x2004, 0x2080);
	BT_u32 MASK[2];		///< Mask register.
	BT_STRUCT_RESERVED_u32(3, 0x2084, 0x2100);
	BT_u32 PIN[2];		///< port pin register.
	BT_STRUCT_RESERVED_u32(4, 0x2104, 0x2180);
	BT_u32 MPIN[2];		///< masked port pin register.
	BT_STRUCT_RESERVED_u32(5, 0x2184, 0x2200);
	BT_u32 SET[2];		///< set register port.
	BT_STRUCT_RESERVED_u32(6, 0x2204, 0x2280);
	BT_u32 CLR[2];		///< clear port.
	BT_STRUCT_RESERVED_u32(7, 0x2284, 0x2300);
	BT_u32 NOT[2];		///< toggle port.
} LPC11Axx_GPIO_REGS;

#endif
