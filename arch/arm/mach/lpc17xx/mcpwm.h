#ifndef _MCPWM_H_
#define _MCPWM_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LPC17xx_MCPWM_oDeviceInterface;

typedef struct _LPC17xx_MCPWM_REGS {
	BT_u32	MCCON;				// 0x000		PWM Control read address
	BT_u32	MCCON_SET;			// 0x004		PWM Control set address
	BT_u32	MCCON_CLR;			// 0x008		PWM Control clear address

#define	LPC17xx_MCPWM_MCCON_RUN					0x00000001
#define	LPC17xx_MCPWM_MCCON_CENTER				0x00000002
#define	LPC17xx_MCPWM_MCCON_POLAR				0x00000004
#define	LPC17xx_MCPWM_MCCON_DTE					0x00000008
#define	LPC17xx_MCPWM_MCCON_DISUP				0x00000010
#define	LPC17xx_MCPWM_MCCON_INVBDC				0x20000000
#define	LPC17xx_MCPWM_MCCON_ACMODE				0x40000000
#define	LPC17xx_MCPWM_MCCON_DCMODE				0x80000000

	BT_u32	MCCAPCON;			// 0x00C		Capture Control read address
	BT_u32	MCCAPCON_SET;		// 0x010		Capture Control set address
	BT_u32	MCCAPCON_CLR;		// 0x014		Event Control clear address
	BT_u32	MCTC[3];			// 0x018		Timer Counter register
	BT_u32	MCLIM[3];			// 0x024		Limit register
	BT_u32	MCMAT[3];			// 0x030		Match register, channel 0
	BT_u32	MCDT;				// 0x03C		Dead time register
	BT_u32	MCCP;				// 0x040		Commutation Pattern register
	BT_u32	MCCAP[3];			// 0x044		Capture register
	BT_u32	MCINTEN;			// 0x050		Interrupt Enable read address
	BT_u32	MCINTEN_SET;		// 0x054		Interrupt Enable set address
	BT_u32	MCINTEN_CLR;		// 0x058		Interrupt Enable clear address

#define	LPC17xx_MCPWM_MCINTEN_ABORT				0x00008000

	BT_u32	MCCNTCON;			// 0x05C		Count Control read address
	BT_u32	MCCNTCON_SET;		// 0x060		Count Control set address
	BT_u32	MCCNTCON_CLR;		// 0x064		Count Control clear address
	BT_u32	MCINTF;				// 0x068		Interrupt flags read address
	BT_u32	MCINTF_SET;			// 0x06C		Interrupt flags set address
	BT_u32	MCINTF_CLR;			// 0x070		Interrupt flags clear address
	BT_u32	MCCAP_CLR;			// 0x074		Capture clear address
} LPC17xx_MCPWM_REGS;

#define MCPWM0						((LPC17xx_MCPWM_REGS *) BT_CONFIG_MACH_LPC17xx_MCPWM0_BASE)

#endif

