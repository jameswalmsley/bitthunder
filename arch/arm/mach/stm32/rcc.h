/**
 *	Defines the Reset and Clock Control Register definitions for STM32
 *
 *
 *
 **/
#ifndef _RCC_H_
#define _RCC_H_

#include <bitthunder.h>

typedef struct _STM32_RCC_REGS {
	BT_u32 CR;
	BT_u32 CFG;
	BT_u32 CI;
	BT_u32 APB2RST;
	BT_u32 APB1RST;
	BT_u32 AHBEN;
	BT_u32 APB2EN;
	BT_u32 APB1EN;
	BT_u32 BDC;
	BT_u32 CSR;
	BT_u32 CFG2;
} STM32_RCC_REGS;

#define STM32_RCC_BASE	0x40021000
#define STM32_RCC 		((STM32_RCC_REGS *) (STM32_RCC_BASE))

#endif
