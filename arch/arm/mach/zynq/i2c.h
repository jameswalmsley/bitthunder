#ifndef _I2C_H_
#define _I2C_H_

#include <bitthunder.h>

typedef struct _ZYNQ_I2C_REGS {
	BT_u32	CONTROL;
    #define CONTROL_RW	0x00000001		///< Master Transfer direction (0 tx, 1 rx).
    #define CONTROL_MS	0x00000002		///< Interface is master.
    #define CONTROL_NEA	0x00000004		///< Interface addressing mode (1 = 7bit).
	BT_u32	STATUS;
	BT_u32	ADDRESS;
	BT_u32	DATA;
	BT_u32	INT_STATUS;
	BT_u32	TxSIZE;
	BT_u32	SLAVE_MON;
	BT_u32	TIMEOUT;
	BT_u32	INT_MASK;
	BT_u32	INT_ENABLE;
	BT_u32	INT_DISABLE;
} ZYNQ_I2C_REGS;










#endif
