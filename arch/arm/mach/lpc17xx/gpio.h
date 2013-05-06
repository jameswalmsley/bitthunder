#ifndef _GPIO_H_
#define _GPIO_H_

#include <bitthunder.h>

typedef struct _LPC17xx_GPIO_BANK {
	BT_u32 FIODIR;
	BT_STRUCT_RESERVED_u32(0, 0x0000, 0x0010);
	BT_u32 FIOMASK;
	BT_u32 FIOPIN;
	BT_u32 FIOSET;
	BT_u32 FIOCLR;
} LPC17xx_GPIO_BANK;

typedef struct _LPC17xx_GPIO_REGS {
	LPC17xx_GPIO_BANK banks[BT_CONFIG_MACH_LPC17xx_TOTAL_GPIOS/32];
} LPC17xx_GPIO_REGS;


#endif
