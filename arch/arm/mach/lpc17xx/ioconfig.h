/**
 *	Defines the Reset and Clock Control Register definitions for STM32
 *
 *
 *
 **/
#ifndef _IOCON_H_
#define _IOCON_H_

#include <bitthunder.h>


typedef struct _LPC17xx_IOCON_REGS {
	BT_u32 LPC17xx_PINSELP[10];
	BT_u32 LPC17xx_PINSEL10;
	BT_STRUCT_RESERVED_u32(0, 0x028, 0x040);
	BT_u32 LPC17xx_PINMODE[10];
	BT_u32 LPC17xx_PINMODE_OD[5];
	BT_u32 LPC17xx_I2CPADCFG;
} LPC17xx_IOCON_REGS;

#define LPC17xx_IOCON_BASE	0x4002C000
#define LPC17xx_IOCON 		((LPC17xx_IOCON_REGS *) (LPC17xx_IOCON_BASE))

#define	LPC17xx_PIO0	&LPC17xx_IOCON->LPC17xx_PINSELP0[0]
#define	LPC17xx_PIO1	&LPC17xx_IOCON->LPC17xx_PINSELP1[0]
#define	LPC17xx_PIO2	&LPC17xx_IOCON->LPC17xx_PINSELP2[0]
#define	LPC17xx_PIO3	&LPC17xx_IOCON->LPC17xx_PINSELP3[0]
#define	LPC17xx_PIO4	&LPC17xx_IOCON->LPC17xx_PINSELP4[0]

void BT_LPC17xx_SetIOConfig(BT_u32 ulIO, BT_u32 ulFunction, BT_u32 ulMode, BT_BOOL bOpenDrain);


#endif

