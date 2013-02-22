#ifndef _GPIO_H_
#define _GPIO_H_

#include <bitthunder.h>

typedef struct _LPC11xx_GPIO_BANK {
	BT_u32 MASK[BT_STRUCT_ARRAY_u32(0x0, 0x3FF8)];
	BT_u32 DATA;
	BT_STRUCT_RESERVED_u32(0, 0x3FFC, 0x8000);
	BT_u32 DIR;		///< Direction register.
	BT_u32 IS; 		///< Interrupt Sense.
	BT_u32 IBE;		///< Interrupt both edges.
	BT_u32 IEV;		///< Interrupt event register.
	BT_u32 IE;		///< Interrupt mask register.
	BT_u32 RIS;		///< Interrupt raw status register.
	BT_u32 MIS;		///< Interrupt masked status register.
	BT_u32 IC;		///< Interrupt clear register.
	BT_STRUCT_RESERVED_u32(1, 0x801C, 0x10000);
} LPC11xx_GPIO_BANK;

typedef struct _LPC11xx_GPIO_REGS {
	LPC11xx_GPIO_BANK banks[BT_CONFIG_MACH_LPC11xx_TOTAL_GPIOS/12];
} LPC11xx_GPIO_REGS;


#endif
