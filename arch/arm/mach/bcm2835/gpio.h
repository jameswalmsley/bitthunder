#ifndef _GPIO_H_
#define _GPIO_H_

typedef struct {
	BT_u32	GPFSEL[6];	///< Function selection registers.
	BT_u32	Reserved_1;
	BT_u32	GPSET[2];
	BT_u32	Reserved_2;
	BT_u32	GPCLR[2];
	BT_u32	Reserved_3;
	BT_u32	GPLEV[2];
	BT_u32	Reserved_4;
	BT_u32	GPEDS[2];
	BT_u32	Reserved_5;
	// etc etc
} BCM2835_GPIO_REGS;


#endif
