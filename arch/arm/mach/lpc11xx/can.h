#ifndef _CAN_H_
#define _CAN_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LPC11xx_CAN_oDeviceInterface;


typedef struct _LPC11xx_CAN_REGS {
	BT_u32	CANCNTL;							//R/W 0x000 CAN control 0x0001
	BT_u32	CANSTAT;							//R/W 0x004 Status register 0x0000
	BT_u32	CANEC;								//RO  0x008 Error counter 0x0000
	BT_u32	CANBT;								//R/W 0x00C Bit timing register 0x2301
	BT_u32	CANINT;								//RO  0x010 Interrupt register 0x0000
	BT_u32	CANTEST;							//R/W 0x014 Test register -
	BT_u32	CANBRPE;							//R/W 0x018 Baud rate prescaler extension register     0x0000
	BT_STRUCT_RESERVED_u32(0, 0x18, 0x20);		//- - 0x01C Reserved -
	BT_u32	CANIF1_CMDREQ;						//R/W 0x020 Message interface 1 command request   0x0001
	BT_u32	CANIF1_CMDMSK;						//R/W 0x024 Message interface 1 command mask
	BT_u32	CANIF1_MSK1;						//R/W 0x028 Message interface 1 mask 1 0xFFFF
	BT_u32	CANIF1_MSK2;						//R/W 0x02C Message interface 1 mask 2 0xFFFF
	BT_u32	CANIF1_ARB1;						//R/W 0x030 Message interface 1 arbitration 1 0x0000
	BT_u32	CANIF1_ARB2;						//R/W 0x034 Message interface 1 arbitration 2 0x0000
	BT_u32	CANIF1_MCTRL;						//R/W 0x038 Message interface 1 message control      0x0000NXP Semiconductors UM10398
	BT_u32	CANIF1_DA1;							//R/W 0x03C Message interface 1 data A1 0x0000
	BT_u32	CANIF1_DA2;							//R/W 0x040 Message interface 1 data A2 0x0000
	BT_u32	CANIF1_DB1;							//R/W 0x044 Message interface 1 data B1 0x0000
	BT_u32	CANIF1_DB2;							//R/W 0x048 Message interface 1 data B2 0x0000
	BT_STRUCT_RESERVED_u32(1, 0x48, 0x80);		//- - 0x04C - 0x07C Reserved -
	BT_u32	CANIF2_CMDREQ;						//R/W 0x080 Message interface 2 command request   0x0001
	BT_u32	CANIF2_CMDMSK;						//R/W 0x084 Message interface 2 command mask
	BT_u32	CANIF2_MSK1;						//R/W 0x088 Message interface 2 mask 1 0xFFFF
	BT_u32	CANIF2_MSK2;						//R/W 0x08C Message interface 2 mask 2 0xFFFF
	BT_u32	CANIF2_ARB1;						//R/W 0x090 Message interface 2 arbitration 1 0x0000
	BT_u32	CANIF2_ARB2;						//R/W 0x094 Message interface 2 arbitration 2 0x0000
	BT_u32	CANIF2_MCTRL;						//R/W 0x098 Message interface 2 message control      0x0000
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
