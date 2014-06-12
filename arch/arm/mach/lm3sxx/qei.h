#ifndef _QEI_H_
#define _QEI_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LM3Sxx_QEI_oDeviceInterface;

typedef struct _LM3Sxx_QEI_REGS {
	BT_u32 	QEICTL;      // 0x000		control register

	#define	LM3Sxx_QEI_QEICTL_FILTER(a)			(a << 16)
	#define	LM3Sxx_QEI_QEICTL_FILTER_ENABLE		0x00002000
	#define	LM3Sxx_QEI_QEICTL_INVI				0x00000800
	#define	LM3Sxx_QEI_QEICTL_INVB				0x00000400
	#define	LM3Sxx_QEI_QEICTL_INVA				0x00000200
	#define	LM3Sxx_QEI_QEICTL_VELEN				0x00000020
	#define	LM3Sxx_QEI_QEICTL_RESMODE			0x00000010
	#define	LM3Sxx_QEI_QEICTL_CAPMODE			0x00000008
	#define	LM3Sxx_QEI_QEICTL_SIGMODE			0x00000004
	#define	LM3Sxx_QEI_QEICTL_SWAP				0x00000002
	#define	LM3Sxx_QEI_QEICTL_ENABLE			0x00000001


	BT_u32 	QEISTAT;     // 0x004       encoder status register

	#define	LM3Sxx_QEI_QEISTAT_DIR				0x00000002

	BT_u32 	QEIPOS;      // 0x008       position register
	BT_u32 	QEIMAXPOS;   // 0x00C       maximum position register
	BT_u32 	QEILOAD;     // 0x010		velocity timer reload register
	BT_u32 	QEITIME;     // 0x014		velocity timer register
	BT_u32 	QEICOUNT;    // 0x018		velocity Counter register
	BT_u32 	QEISPEED;    // 0x01C		velocity register
	BT_u32 	QEIINTEN;    // 0x020		velocity counter register
	BT_u32 	QEIRIS;      // 0x024		raw interrupt status
	BT_u32 	QEIISC;      // 0x028		interrupt status and clear

} LM3Sxx_QEI_REGS;

#define QEI0						((LM3Sxx_QEI_REGS *) BT_CONFIG_MACH_LM3Sxx_QEI0_BASE)
#define QEI1						((LM3Sxx_QEI_REGS *) BT_CONFIG_MACH_LM3Sxx_QEI1_BASE)

#endif

