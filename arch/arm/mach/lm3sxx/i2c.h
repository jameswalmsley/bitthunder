#ifndef _I2C_H_
#define _I2C_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LM3Sxx_I2C_oDeviceInterface;


typedef struct _LM3Sxx_I2C_REGS {
	BT_u32	LM3Sxx_I2C_MSA;					//		0x00  -- Master Slave Address

#define	LM3Sxx_I2C_MASTER_SA_RS					0x00000001

	BT_u32	LM3Sxx_I2C_MCS;					//		0x04  -- Master Status Register.

#define	LM3Sxx_I2C_MCS_RUN						0x00000001
#define	LM3Sxx_I2C_MCS_START					0x00000002
#define	LM3Sxx_I2C_MCS_STOP						0x00000004
#define	LM3Sxx_I2C_MCS_ADRACK					0x00000004
#define	LM3Sxx_I2C_MCS_DATACK					0x00000008
#define	LM3Sxx_I2C_MCS_BUSBSY					0x00000040

	BT_u32	LM3Sxx_I2C_MDR;					//		0x08  -- Master Data register
	BT_u32	LM3Sxx_I2C_MTPR;				//		0x0C  -- Master Timer Period
	BT_u32	LM3Sxx_I2C_MIMR;				//		0x10  -- Master Interrupt Mask
	BT_u32	LM3Sxx_I2C_MRIS;				//		0x14  -- Master Raw Interrupt Status

#define	LM3Sxx_I2C_MRIS_RIS						0x00000001

	BT_u32	LM3Sxx_I2C_MMIS;				//		0x18  -- Master Masked Interrupt Status
	BT_u32	LM3Sxx_I2C_MICR;				//		0x1C  -- Master Interrupt Clear

#define	LM3Sxx_I2C_MICR_IC						0x00000001

	BT_u32	LM3Sxx_I2C_MCR;					//		0x20  -- Master Configuration

#define	LM3Sxx_I2C_MCR_LOOPBACK_MODE			0x00000001
#define	LM3Sxx_I2C_MCR_MASTER_MODE				0x00000010
#define	LM3Sxx_I2C_MCR_SLAVE_MODE				0x00000020

	BT_STRUCT_RESERVED_u32(0, 0x20, 0x800);

	BT_u32	LM3Sxx_I2C_SOAR;				//		0x800  -- Slave Address
	BT_u32	LM3Sxx_I2C_SCSR;				//		0x804  -- Slave Status Register
	BT_u32	LM3Sxx_I2C_SDR;					//		0x808  -- Slave Data register
	BT_u32	LM3Sxx_I2C_SIMR;				//		0x80C  -- Slave Interrupt Mask
	BT_u32	LM3Sxx_I2C_SRIS;				//		0x810  -- Slave Raw Interupt Status
	BT_u32	LM3Sxx_I2C_SMIS;				//		0x814  -- Slave Masked Interupt Status
	BT_u32	LM3Sxx_I2C_SICR;				//		0x818  -- Slave Interrupt Clear
} LM3Sxx_I2C_REGS;

#define I2C0						((LM3Sxx_I2C_REGS *) BT_CONFIG_MACH_LM3Sxx_I2C0_BASE)
#define I2C1						((LM3Sxx_I2C_REGS *) BT_CONFIG_MACH_LM3Sxx_I2C1_BASE)



#endif
