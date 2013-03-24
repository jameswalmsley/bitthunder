#ifndef _TIMER_H_
#define _TIMER_H_

typedef struct _BCM2835_TIMER_REGS {
	unsigned long LOD;
	unsigned long VAL;
	unsigned long CTL;
	#define CTL_ENABLE			0x00000080
	#define CTL_INT_ENABLE		0x00000020
	unsigned long CLI;
	unsigned long RIS;
	unsigned long MIS;
	unsigned long RLD;
	unsigned long DIV;
	#define DIV_VAL				0x000001FF
	unsigned long CNT;
} BCM2835_TIMER_REGS;



#endif
