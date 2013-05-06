#ifndef _CAN_H_
#define _CAN_H_

#include <bitthunder.h>
#include <bt_struct.h>

#define	LPC17xx_CAN_NO_FREE_BUFFER				0xFFFFFFFF

const BT_IF_DEVICE BT_LPC17xx_CAN_oDeviceInterface;

typedef struct _LPC17xx_CAN_BUFFER{
	BT_u32	CANFS;

#define	LPC17xx_CAN_FS_ID(x)					((x & 0x000003FF))
#define	LPC17xx_CAN_FS_BP						0x00000400
#define	LPC17xx_CAN_FS_DLC(x)					((x & 0x000F0000)>>16)
#define	LPC17xx_CAN_FS_RTR						0x40000000
#define	LPC17xx_CAN_FS_FF						0x80000000

	BT_u32	CANID;
	BT_u32	CANData[2];
} LPC17xx_CAN_BUFFER;

typedef struct _LPC17xx_CAN_COMMON_REGS {
	BT_u32 LPC17xx_CAN_AFMR;

#define LPC17xx_CAN_AFMR_ACCOFF					0x00000001
#define LPC17xx_CAN_AFMR_ACCBP					0x00000002
#define LPC17xx_CAN_AFMR_eFCAN					0x00000004

	BT_u32 LPC17xx_CAN_SFF_sa;
	BT_u32 LPC17xx_CAN_SFF_GRP_sa;
	BT_u32 LPC17xx_CAN_EFF_sa;
	BT_u32 LPC17xx_CAN_EFF_GRP_sa;
	BT_u32 LPC17xx_CAN_ENDofTable;
	BT_u32 LPC17xx_CAN_LUTerrAd;
	BT_u32 LPC17xx_CAN_LUTerr;
	BT_STRUCT_RESERVED_u32(0, 0x3C01C, 0x40000);
	BT_u32 LPC17xx_CAN_TxSR;
	BT_u32 LPC17xx_CAN_RxSR;
	BT_u32 LPC17xx_CAN_MSR;
} LPC17xx_CAN_COMMON_REGS;


typedef struct _LPC17xx_CAN_REGS {
	BT_u32				CANMOD;					//R/W 0x000 mode register

#define	LPC17xx_CAN_MOD_RM						0x00000001

	BT_u32				CANCMR;					//R/W 0x004 command register 0x0000

#define	LPC17xx_CAN_CMR_TR						0x00000001
#define	LPC17xx_CAN_CMR_RRB						0x00000004
#define	LPC17xx_CAN_CMR_SELECT_TXBUF			0x00000020


	BT_u32				CANGSR;					//RO  0x008 global controller status and Error counters 0x0000

#define	LPC17xx_CAN_GSR_RBS						0x00000001
#define	LPC17xx_CAN_GSR_TBS						0x00000004
#define	LPC17xx_CAN_GSR_TCS						0x00000008
#define	LPC17xx_CAN_GSR_RS						0x00000010
#define	LPC17xx_CAN_GSR_TS						0x00000020
#define	LPC17xx_CAN_GSR_BS						0x00000040

	BT_u32				CANICR;					//R/W 0x00C Interrupt Status register 0x0000

#define	LPC17xx_CAN_ICR_RI						0x00000001
#define	LPC17xx_CAN_ICR_TI1						0x00000002
#define	LPC17xx_CAN_ICR_TI2						0x00000200
#define	LPC17xx_CAN_ICR_TI3						0x00000400
#define	LPC17xx_CAN_ICR_TI						0x00000602

	BT_u32				CANIER;					//RO  0x010 Interrupt Enable register 0x0000

#define	LPC17xx_CAN_IER_RIE						0x00000001
#define	LPC17xx_CAN_IER_TIE1					0x00000002
#define	LPC17xx_CAN_IER_TIE2					0x00000200
#define	LPC17xx_CAN_IER_TIE3					0x00000400
#define	LPC17xx_CAN_IER_TIE						0x00000602

	BT_u32				CANBTR;					//R/W 0x014 Bit timing register
	BT_u32				CANBEWL;				//R/W 0x018 error warning limit register
	BT_u32				CANSR;					//R/W 0x01C status register

#define	LPC17xx_CAN_SR_TBS1						0x00000004
#define	LPC17xx_CAN_SR_TCS1						0x00000008
#define	LPC17xx_CAN_SR_TS1						0x00000020
#define	LPC17xx_CAN_SR_TBS2						0x00000400
#define	LPC17xx_CAN_SR_TCS2						0x00000800
#define	LPC17xx_CAN_SR_TS2						0x00002000
#define	LPC17xx_CAN_SR_TBS3						0x00040000
#define	LPC17xx_CAN_SR_TCS3						0x00080000
#define	LPC17xx_CAN_SR_TS3						0x00200000

	LPC17xx_CAN_BUFFER	CANRCVBuf[1];			//R/W 0x020 receive frame status
	LPC17xx_CAN_BUFFER	CANTMTBuf[3];			//R/W 0x030
} LPC17xx_CAN_REGS;



#define CAN_COMMON					((LPC17xx_CAN_COMMON_REGS *) 0x4003C000)
#define CAN0						((LPC17xx_CAN_REGS *) BT_CONFIG_MACH_LPC17xx_CAN0_BASE)
#define CAN1						((LPC17xx_CAN_REGS *) BT_CONFIG_MACH_LPC17xx_CAN1_BASE)

#endif
