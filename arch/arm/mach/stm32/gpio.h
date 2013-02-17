#ifndef _GPIO_H_
#define _GPIO_H_

#include <bitthunder.h>

typedef struct _STM32_GPIO_BANK {
	BT_u32 CR[2];	///< Configuration Low and High!
	BT_u32 IDR; 	///< Input data register.
	BT_u32 ODR;		///< Output data register.
	BT_u32 BSRR;	///< Bit set/reset register.
	BT_u32 BRR;		///< Bit reset register.
	BT_u32 LCKR;	///< Configuration lock register.
	BT_STRUCT_RESERVED_u32(0, 0x18, 0x400);
} STM32_GPIO_BANK;

typedef struct _STM32_GPIO_REGS {
	STM32_GPIO_BANK banks[BT_CONFIG_MACH_STM32_TOTAL_GPIOS/16];
} STM32_GPIO_REGS;


#endif
