/**
 *	ARM Cortex-Mx Systick Peripheral Driver.
 *
 *	@author James Walmsley
 **/
#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include <bitthunder.h>


typedef struct _SYSTICK_REGS {
	BT_u32 	CTRL;
#define SYSTICK_CTRL_ENABLE 	0x00000001
#define SYSTICK_CTRL_TICKINT 	0x00000002
#define SYSTICK_CTRL_CLKSOURCE 	0x00000004
#define SYSTICK_CTRL_COUNTFLAG 	0x00010000

	BT_u32	LOAD;
	BT_u32	VALUE;
	BT_u32	CALIB;
} SYSTICK_REGS;


#endif
