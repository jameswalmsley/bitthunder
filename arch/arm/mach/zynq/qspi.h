#ifndef _QSPI_H_
#define _QSPI_H_

#include <bitthunder.h>

#define ZYNQ_QSPI_CONTROLLER_BASE		0xE000D000

typedef struct _ZYNQ_QSPI_REGS {
	BT_u32 CONFIG;						/* Configuration  Register, RW */
	/*
	 * QSPI Configuration Register bit Masks
	 *
	 * This register contains various control bits that effect the operation
	 * of the QSPI controller
	 */
	#define QSPI_CONFIG_MANSRT_MASK      0x00010000 /* Manual TX Start */
	#define QSPI_CONFIG_CPHA_MASK        0x00000004 /* Clock Phase Control */
	#define QSPI_CONFIG_CPOL_MASK        0x00000002 /* Clock Polarity Control */
	#define QSPI_CONFIG_SSCTRL_MASK      0x00003C00 /* Slave Select Mask */

	BT_u32 INT_STATUS;					/* Interrupt Status Register, RO */
	BT_u32 INT_ENABLE;					/* Interrupt Enable Register, WO */
	BT_u32 INT_DISABLE;					/* Interrupt Disable Reg, WO */
	BT_u32 INT_MASK;					/* Interrupt Enabled Mask Reg,RO */
	/*
	 * QSPI Interrupt Registers bit Masks
	 *
	 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
	 * bit definitions.
	 */
	#define QSPI_IXR_TXNFULL_MASK        0x00000004 /* QSPI TX FIFO Overflow */
	#define QSPI_IXR_TXFULL_MASK         0x00000008 /* QSPI TX FIFO is full */
	#define QSPI_IXR_RXNEMTY_MASK        0x00000010 /* QSPI RX FIFO Not Empty */
	#define QSPI_IXR_ALL_MASK            (QSPI_IXR_TXNFULL_MASK)


	BT_u32 ENABLE;						/* Enable/Disable Register, RW */
	/*
	 * QSPI Enable Register bit Masks
	 *
	 * This register is used to enable or disable the QSPI controller
	 */
	#define QSPI_ENABLE_ENABLE_MASK      0x00000001 /* QSPI Enable Bit Mask */


	BT_u32 DELAY;						/* Delay Register, RW */
	BT_u32 TXD_00_00;					/* Transmit 4-byte inst, WO */
	BT_u32 RXD;							/* Data Receive Register, RO */
	BT_u32 SIC;							/* Slave Idle Count Register, RW */
	BT_u32 TX_THRESH;					/* TX FIFO Watermark Reg, RW */
	BT_u32 RX_THRESH;					/* RX FIFO Watermark Reg, RW */
	BT_u32 GPIO;						/* GPIO Register, RW */

	BT_STRUCT_RESERVED_u32(0, 0x30, 0x38);

	BT_u32 LPBK_DLY_ADJ;					/* Loopback master clock delay adjustment */

	BT_STRUCT_RESERVED_u32(1, 0x38, 0x80);

	BT_u32 TXD_00_01;					/* Transmit 1-byte inst, WO */
	BT_u32 TXD_00_10;					/* Transmit 2-byte inst, WO */
	BT_u32 TXD_00_11;					/* Transmit 3-byte inst, WO */

	BT_STRUCT_RESERVED_u32(2, 0x88, 0xA0);

	BT_u32 LINEAR_CFG;					/* Linear Adapter Config Ref, RW */
	/*
	 * QSPI Linear Configuration Register
	 *
	 * It is named Linear Configuration but it controls other modes when not in
	 * linear mode also.
	 */
	#define QSPI_LCFG_TWO_MEM_MASK       0x40000000 /* LQSPI Two memories Mask */
	#define QSPI_LCFG_SEP_BUS_MASK       0x20000000 /* LQSPI Separate bus Mask */
	#define QSPI_LCFG_U_PAGE_MASK        0x10000000 /* LQSPI Upper Page Mask */

	#define QSPI_LCFG_DUMMY_SHIFT        8

	#define QSPI_FAST_READ_QOUT_CODE     0x6B    /* read instruction code */

	BT_u32 LINEAR_STATUS;				/* Linear Adapter Status Reg */

	BT_STRUCT_RESERVED_u32(3, 0xA4, 0xFC);

	BT_u32 MOD_ID;						/* Module ID Register, RO */

} ZYNQ_QSPI_REGS;


#endif
