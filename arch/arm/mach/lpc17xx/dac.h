#ifndef _DAC_H_
#define _DAC_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LPC17xx_DAC_oDeviceInterface;

typedef struct _LPC17xx_DAC_REGS {
	BT_u32	DACR;					// 0x00		converter register

#define	LPC17xx_DAC_DACR_BIAS					0x00010000

	BT_u32	DACCTRL;					// 0x04		control register

#define	LPC17xx_DAC_DACCTRL_INT_DMA_REQ			0x00000001
#define	LPC17xx_DAC_DACCTRL_DBLBUF_ENB			0x00000002
#define	LPC17xx_DAC_DACCTRL_CNT_ENB				0x00000004
#define	LPC17xx_DAC_DACCTRL_DMA_ENB				0x00000008

	BT_u32	DACCNTVAL;					// 0x08		DAC interrupt interval

} LPC17xx_DAC_REGS;

#define DAC0						((LPC17xx_DAC_REGS *) BT_CONFIG_MACH_LPC17xx_DAC0_BASE)

#endif

