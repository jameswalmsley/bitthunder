#ifndef _CAN_H_
#define _CAN_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LPC11xx_CAN_oDeviceInterface;

#define	LPC11xx_CAN_DLC_MAX						8

typedef struct _LPC11xx_CAN_REGS {
	BT_u32	CANCNTL;							//R/W 0x000 CAN control 0x0001

#define	LPC11xx_CAN_CTRL_INIT					0x0001
#define	LPC11xx_CAN_CTRL_IE						0x0002
#define	LPC11xx_CAN_CTRL_SIE					0x0004
#define	LPC11xx_CAN_CTRL_EIE					0x0008
#define	LPC11xx_CAN_CTRL_CCE					0x0040
#define	LPC11xx_CAN_CTRL_TEST					0x0080

	BT_u32	CANSTAT;							//R/W 0x004 Status register 0x0000

#define LPC11xx_CAN_STAT_LEC					0x0007
#define LPC11xx_CAN_STAT_TXOK					0x0008
#define LPC11xx_CAN_STAT_RXOK					0x0010
#define LPC11xx_CAN_STAT_EPASS					0x0020
#define LPC11xx_CAN_STAT_EWARN					0x0040
#define LPC11xx_CAN_STAT_BOFF					0x0080

	BT_u32	CANEC;								//RO  0x008 Error counter 0x0000
	BT_u32	CANBT;								//R/W 0x00C Bit timing register 0x2301
	BT_u32	CANINT;								//RO  0x010 Interrupt register 0x0000

#define	LPC11xx_CAN_INT_STATUS_INTERRUPT		0x8000

	BT_u32	CANTEST;							//R/W 0x014 Test register -
	BT_u32	CANBRPE;							//R/W 0x018 Baud rate prescaler extension register     0x0000
	BT_STRUCT_RESERVED_u32(0, 0x18, 0x20);		//- - 0x01C Reserved -
	BT_u32	CANIF1_CMDREQ;						//R/W 0x020 Message interface 1 command request   0x0001
	BT_u32	CANIF1_CMDMSK;						//R/W 0x024 Message interface 1 command mask
	BT_u32	CANIF1_MSK1;						//R/W 0x028 Message interface 1 mask 1 0xFFFF
	BT_u32	CANIF1_MSK2;						//R/W 0x02C Message interface 1 mask 2 0xFFFF
	BT_u32	CANIF1_ARB1;						//R/W 0x030 Message interface 1 arbitration 1 0x0000
	BT_u32	CANIF1_ARB2;						//R/W 0x034 Message interface 1 arbitration 2 0x0000

#define	LPC11xx_CAN_IFARB2_ID_MVAL		(1 << 15)     /* Message valid bit, 1 is valid in the MO handler, 0 is ignored */
#define	LPC11xx_CAN_IFARB2_ID_MTD		(1 << 14)     /* 1 extended identifier bit is used in the RX filter unit, 0 is not */
#define	LPC11xx_CAN_IFARB2_ID_DIR		(1 << 13)     /* 1 direction bit is used in the RX filter unit, 0 is not */

	BT_u32	CANIF1_MCTRL;						//R/W 0x038 Message interface 1 message control      0x0000NXP Semiconductors UM10398
	BT_u32	CANIF1_DA1;							//R/W 0x03C Message interface 1 data A1 0x0000
	BT_u32	CANIF1_DA2;							//R/W 0x040 Message interface 1 data A2 0x0000
	BT_u32	CANIF1_DB1;							//R/W 0x044 Message interface 1 data B1 0x0000
	BT_u32	CANIF1_DB2;							//R/W 0x048 Message interface 1 data B2 0x0000
	BT_STRUCT_RESERVED_u32(1, 0x48, 0x80);		//- - 0x04C - 0x07C Reserved -
	BT_u32	CANIF2_CMDREQ;						//R/W 0x080 Message interface 2 command request   0x0001

#define LPC11xx_CAN_IFCREQ_BUSY          	     0x8000

	BT_u32	CANIF2_CMDMSK;						//R/W 0x084 Message interface 2 command mask

	/* bit field of IF command mask register */
#define	LPC11xx_CAN_IFCMDMSK_DATAB				(1 << 0)   /* 1 is transfer data byte 4-7 to message object, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_DATAA				(1 << 1)   /* 1 is transfer data byte 0-3 to message object, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_TREQ				(1 << 2)   /* 1 is set the TxRqst bit, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_INTPND				(1 << 3)
#define	LPC11xx_CAN_IFCMDMSK_CTRL				(1 << 4)   /* 1 is transfer the CTRL bit to the message object, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_ARB				(1 << 5)   /* 1 is transfer the ARB bits to the message object, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_MASK				(1 << 6)   /* 1 is transfer the MASK bit to the message object, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_WR					(1 << 7)   /* 0 is READ, 1 is WRITE */
#define LPC11xx_CAN_IFCMDMSK_RD    				0x0000

#define LPC11xx_CAN_IFCMDMSK_ID_STD_MASK		0x07FF
#define LPC11xx_CAN_IFCMDMSK_ID_EXT_MASK		0x1FFFFFFF
#define LPC11xx_CAN_IFCMDMSK_DLC_MASK			0x0F

#define	LPC11xx_CAN_IFCMDMSK_MASK_MXTD			0x8000     /* 1 extended identifier bit is used in the RX filter unit, 0 is not */
#define	LPC11xx_CAN_IFCMDMSK_MASK_MDIR			0x4000     /* 1 direction bit is used in the RX filter unit, 0 is not */

	BT_u32	CANIF2_MSK1;						//R/W 0x088 Message interface 2 mask 1 0xFFFF
	BT_u32	CANIF2_MSK2;						//R/W 0x08C Message interface 2 mask 2 0xFFFF
	BT_u32	CANIF2_ARB1;						//R/W 0x090 Message interface 2 arbitration 1 0x0000
	BT_u32	CANIF2_ARB2;						//R/W 0x094 Message interface 2 arbitration 2 0x0000

#define	LPC11xx_CAN_ID_MVAL						0x8000     /* Message valid bit, 1 is valid in the MO handler, 0 is ignored */
#define	LPC11xx_CAN_ID_MTD						0x4000     /* 1 extended identifier bit is used in the RX filter unit, 0 is not */
#define	LPC11xx_CAN_ID_DIR						0x2000     /* 1 direction bit is used in the RX filter unit, 0 is not */

	BT_u32	CANIF2_MCTRL;						//R/W 0x098 Message interface 2 message control      0x0000

#define	LPC11xx_CANIFMCTRL_NEWD					0x8000     /* 1 indicates new data is in the message buffer.  */
#define	LPC11xx_CANIFMCTRL_MLST					0x4000     /* 1 indicates a message loss. */
#define	LPC11xx_CANIFMCTRL_INTP					0x2000     /* 1 indicates message object is an interrupt source */
#define LPC11xx_CANIFMCTRL_UMSK			    	0x1000     /* 1 is to use the mask for the receive filter mask. */
#define	LPC11xx_CANIFMCTRL_TXIE					0x0800     /* 1 is TX interrupt enabled */
#define	LPC11xx_CANIFMCTRL_RXIE					0x0400     /* 1 is RX interrupt enabled */
#define	LPC11xx_CANIFMCTRL_ROEN					0x0200     /* 1 is remote frame enabled */
#define LPC11xx_CANIFMCTRL_TXRQ			    	0x0100     /* 1 is TxRqst enabled */
#define	LPC11xx_CANIFMCTRL_EOB					0x0080     /* End of buffer, always write to 1 */
#define	LPC11xx_CANIFMCTRL_DLC					0x000F        /* bit mask for DLC */

	BT_u32	CANIF2_DA1;							//R/W 0x09C Message interface 2 data A1 0x0000
	BT_u32	CANIF2_DA2;							//R/W 0x0A0 Message interface 2 data A2 0x0000
	BT_u32	CANIF2_DB1;							//R/W 0x0A4 Message interface 2 data B1 0x0000
	BT_u32	CANIF2_DB2;							//R/W 0x0A8 Message interface 2 data B2 0x0000
	BT_STRUCT_RESERVED_u32(2, 0xa8, 0x100);		//- - 0x0AC - 0x0FC Reserved -
	BT_u32	CANTXREQ1;							//RO  0x100 Transmission request 1 0x0000
	BT_u32	CANTXREQ2;							//RO  0x104 Transmission request 2 0x0000
	BT_STRUCT_RESERVED_u32(3, 0x104, 0x120);	//- - 0x108 - 0x11C Reserved -
	BT_u32	CANND1;								//RO  0x120 New data 1 0x0000
	BT_u32	CANND2;								//RO  0x124 New data 2 0x0000
	BT_STRUCT_RESERVED_u32(4, 0x124, 0x140);	//- - 0x128 -	0x13C Reserved -
	BT_u32	CANIR1;								//RO  0x140 Interrupt pending 1 0x0000
	BT_u32	CANIR2;								//RO  0x144 Interrupt pending 2 0x0000
	BT_STRUCT_RESERVED_u32(5, 0x144, 0x160);	//- - 0x148 - 0x15C Reserved -
	BT_u32	CANMSGV1;							//RO  0x160 Message valid 1 0x0000
	BT_u32	CANMSGV2;							//RO  0x164 Message valid 2 0x0000
	BT_STRUCT_RESERVED_u32(6, 0x164, 0x180);	//- - 0x168 -	0x17C Reserved -
	BT_u32	CANCLKDIV;							//R/W 0x180 Can clock divider register 0x0000
} LPC11xx_CAN_REGS;

#define CAN0						((LPC11xx_CAN_REGS *) BT_CONFIG_MACH_LPC11xx_CAN0_BASE)


/*
 * Implementing ROM CAN driver
 */
// error status bits
#define LPC11xx_CAN_ERROR_NONE  0x00000000
#define LPC11xx_CAN_ERROR_PASS  0x00000001
#define LPC11xx_CAN_ERROR_WARN  0x00000002
#define LPC11xx_CAN_ERROR_BOFF  0x00000004
#define LPC11xx_CAN_ERROR_STUF  0x00000008
#define LPC11xx_CAN_ERROR_FORM  0x00000010
#define LPC11xx_CAN_ERROR_ACK   0x00000020
#define LPC11xx_CAN_ERROR_BIT1  0x00000040
#define LPC11xx_CAN_ERROR_BIT0  0x00000080
#define LPC11xx_CAN_ERROR_CRC   0x00000100

// control bits for CAN_MSG_OBJ.mode_id
#define LPC11xx_CAN_MSGOBJ_STD  0x00000000   // CAN 2.0a 11-bit ID
#define LPC11xx_CAN_MSGOBJ_EXT  0x20000000   // CAN 2.0b 29-bit ID
#define LPC11xx_CAN_MSGOBJ_DAT  0x00000000   // data frame
#define LPC11xx_CAN_MSGOBJ_RTR  0x40000000   // rtr frame

typedef struct _LPC11xx_CAN_MSG_OBJ {
	BT_u32	ulmodeid;
	BT_u32	ulmask;
	BT_u8	ucdata[8];
	BT_u8	uclength;
	BT_u8	ucmsgobj;
} LPC11xx_CAN_MSG_OBJ;

/**************************************************************************
SDO Abort Codes
**************************************************************************/
#define LPC11xx_CAN_SDO_ABORT_TOGGLE          0x05030000  // Toggle bit not alternated
#define LPC11xx_CAN_SDO_ABORT_SDOTIMEOUT      0x05040000  // SDO protocol timed out
#define LPC11xx_CAN_SDO_ABORT_UNKNOWN_COMMAND 0x05040001  // Client/server command specifier not valid or unknown
#define LPC11xx_CAN_SDO_ABORT_UNSUPPORTED     0x06010000  // Unsupported access to an object
#define LPC11xx_CAN_SDO_ABORT_WRITEONLY       0x06010001  // Attempt to read a write only object
#define LPC11xx_CAN_SDO_ABORT_READONLY        0x06010002  // Attempt to write a read only object
#define LPC11xx_CAN_SDO_ABORT_NOT_EXISTS      0x06020000  // Object does not exist in the object dictionary
#define LPC11xx_CAN_SDO_ABORT_PARAINCOMP      0x06040043  // General parameter incompatibility reason
#define LPC11xx_CAN_SDO_ABORT_ACCINCOMP       0x06040047  // General internal incompatibility in the device
#define LPC11xx_CAN_SDO_ABORT_TYPEMISMATCH    0x06070010  // Data type does not match, length of service parameter does not match
#define LPC11xx_CAN_SDO_ABORT_UNKNOWNSUB      0x06090011  // Sub-index does not exist
#define LPC11xx_CAN_SDO_ABORT_VALUE_RANGE     0x06090030  // Value range of parameter exceeded (only for write access)
#define LPC11xx_CAN_SDO_ABORT_TRANSFER        0x08000020  // Data cannot be transferred or stored to the application
#define LPC11xx_CAN_SDO_ABORT_LOCAL           0x08000021  // Data cannot be transferred or stored to the application because of local control
#define LPC11xx_CAN_SDO_ABORT_DEVSTAT         0x08000022  // Data cannot be transferred or stored to the application because of the present device state

typedef struct _LPC11xx_CAN_ODCONSTENTRY {
	BT_u16	usindex;
	BT_u8	ucsubindex;
	BT_u8	uclen;
	BT_u32	ulval;
} LPC11xx_CAN_ODCONSTENTRY;

// upper-nibble values for CAN_ODENTRY.entrytype_len
#define LPC11xx_OD_NONE    0x00    // Object Dictionary entry doesn't exist
#define LPC11xx_OD_EXP_RO  0x10    // Object Dictionary entry expedited, read-only
#define LPC11xx_OD_EXP_WO  0x20    // Object Dictionary entry expedited, write-only
#define LPC11xx_OD_EXP_RW  0x30    // Object Dictionary entry expedited, read-write
#define LPC11xx_OD_SEG_RO  0x40    // Object Dictionary entry segmented, read-only
#define LPC11xx_OD_SEG_WO  0x50    // Object Dictionary entry segmented, write-only
#define LPC11xx_OD_SEG_RW  0x60    // Object Dictionary entry segmented, read-write

typedef struct _LPC11xx_CAN_ODENTRY {
	BT_u16 usindex;
	BT_u8  ucsubindex;
	BT_u8  ucentrytype_len;
	BT_u8 *pval;
} LPC11xx_CAN_ODENTRY;

typedef struct _LPC11xx_CAN_CANOPENCFG {
	BT_u8						ucnode_id;
	BT_u8						ucmsgobj_rx;
	BT_u8						ucmsgobj_tx;
	BT_u8						ucisr_handled;
	BT_u32						ulod_const_num;
	LPC11xx_CAN_ODCONSTENTRY   *pod_const_table;
	BT_u32						ulod_num;
	LPC11xx_CAN_ODENTRY		   *pod_table;
} LPC11xx_CAN_CANOPENCFG;

// Return values for CANOPEN_sdo_req() callback
#define LPC11xx_CAN_SDOREQ_NOTHANDLED     0  // process regularly, no impact
#define LPC11xx_CAN_SDOREQ_HANDLED_SEND   1  // processed in callback, auto-send returned msg
#define LPC11xx_CAN_SDOREQ_HANDLED_NOSEND 2  // processed in callback, don't send response

// Values for CANOPEN_sdo_seg_read/write() callback 'openclose' parameter
#define LPC11xx_CAN_SDOSEG_SEGMENT        0  // segment read/write
#define LPC11xx_CAN_SDOSEG_OPEN           1  // channel is opened
#define LPC11xx_CAN_SDOSEG_CLOSE          2  // channel is closed



#endif
