#ifndef _QEI_H_
#define _QEI_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LPC17xx_QEI_oDeviceInterface;

typedef struct _LPC17xx_QEI_REGS {
	BT_u32 	QEICON;      // 0x000		control register
	BT_u32 	QEISTAT;     // 0x004       encoder status register
	BT_u32 	QEICONF;     // 0x008		configuration register

#define	LPC17xx_QEI_QEICONF_DIRINV			0x00000001
#define	LPC17xx_QEI_QEICONF_SIGMODE			0x00000002
#define	LPC17xx_QEI_QEICONF_CAPMODE			0x00000004

	BT_u32 	QEIPOS;      // 0x00C       position register
	BT_u32 	QEIMAXPOS;   // 0x010       maximum position register
	BT_u32 	CMPOS[3];    // 0x014       position compare register 0..3
	BT_u32 	INXCNT;      // 0x020		index count register
	BT_u32 	INXCMP;      // 0x024		index compare register
	BT_u32 	QEILOAD;     // 0x028		velocity timer reload register
	BT_u32 	QEITIME;     // 0x02C		velocity timer register
	BT_u32 	QEIVEL;      // 0x030		velocity counter register
	BT_u32 	QEICAP;      // 0x034		velocity capture register
	BT_u32 	VELCOMP;     // 0x038		velocity compare register
	BT_u32 	FILTER;      // 0x03C		data filter register
	BT_STRUCT_RESERVED_u32(0, 0x03C, 0xFD8);
	BT_u32 	QEIIEC;      // 0xFD8		interrupt enable clear register
	BT_u32 	QEIIES;      // 0xFDC		interrupt enable set register
	BT_u32 	QEIINTSTAT;  // 0xFE0		interrupt status register
	BT_u32 	QEIIE;       // 0xFE4		interrupt enable register
	BT_u32 	QEICLR;      // 0xFE8		interrupt status clear register
	BT_u32 	QEISET;      // 0xFEC		interrupt status set register
} LPC17xx_QEI_REGS;

#define QEI0						((LPC17xx_QEI_REGS *) BT_CONFIG_MACH_LPC17xx_QEI0_BASE)

#endif

