#ifndef _I2C_H_
#define _I2C_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LPC11xx_I2C_oDeviceInterface;


typedef struct _LPC11xx_I2C_REGS {
	BT_u32	LPC11xx_I2C_CONSET;				//		0x00  -- Control Set Register.

#define	LPC11xx_I2C_CONSET_AA				0x00000004
#define	LPC11xx_I2C_CONSET_SI				0x00000008
#define	LPC11xx_I2C_CONSET_STO				0x00000010
#define	LPC11xx_I2C_CONSET_STA				0x00000020
#define	LPC11xx_I2C_CONSET_I2EN				0x00000040

	BT_u32	LPC11xx_I2C_STAT;				//		0x04  -- Status Register.

#define	LPC11xx_I2C_STAT_START_TRANSMITTED				0x00000008
#define	LPC11xx_I2C_STAT_REPEAT_START_TRANSMITTED		0x00000010
#define	LPC11xx_I2C_STAT_ADDRESS_W_ACK					0x00000018
#define	LPC11xx_I2C_STAT_ADDRESS_W_NOT_ACK				0x00000020
#define	LPC11xx_I2C_STAT_DATA_W_ACK						0x00000028
#define	LPC11xx_I2C_STAT_DATA_W_NOT_ACK					0x00000030
#define	LPC11xx_I2C_STAT_ADDRESS_R_ACK					0x00000040
#define	LPC11xx_I2C_STAT_ADDRESS_R_NOT_ACK				0x00000048
#define	LPC11xx_I2C_STAT_DATA_R_ACK						0x00000050
#define	LPC11xx_I2C_STAT_DATA_R_NOT_ACK					0x00000058


	BT_u32	LPC11xx_I2C_DAT;				//		0x08  -- Data register
	BT_u32	LPC11xx_I2C_ADR0;				//		0x0C  -- Slave Address Register 0. Contains the 7-bit
	BT_u32	LPC11xx_I2C_SCLH;				//		0x10  -- SCH Duty Cycle Register High Half Word.
	BT_u32	LPC11xx_I2C_SCLL;				//		0x14  -- SCL Duty Cycle Register Low Half Word.
	BT_u32	LPC11xx_I2C_CONCLR;				//		0x18  -- Control Clear Register.

#define	LPC11xx_I2C_CONCLR_AA				0x00000004
#define	LPC11xx_I2C_CONCLR_SI				0x00000008
#define	LPC11xx_I2C_CONCLR_STO				0x00000010
#define	LPC11xx_I2C_CONCLR_STA				0x00000020
#define	LPC11xx_I2C_CONCLR_I2EN				0x00000040

	BT_u32	LPC11xx_I2C_MMCTRL;				//		0x1C  -- Monitor mode control register.
	BT_u32	LPC11xx_I2C_ADR1;				//		0x20  -- Slave Address Register 1. Contains the 7-bit
	BT_u32	LPC11xx_I2C_ADR2;				//		0x24  -- Slave Address Register 2. Contains the 7-bit
	BT_u32	LPC11xx_I2C_ADR3;				//		0x28  -- Slave Address Register 3. Contains the 7-bit
	BT_u32	LPC11xx_I2C_DATA_BUFFER;		//		0x2C  --
	BT_u32	LPC11xx_I2C_MASK0;				//		0x30  -- Slave address mask register 0. This mask
	BT_u32	LPC11xx_I2C_MASK1;				//		0x34  -- Slave address mask register 1. This mask
	BT_u32	LPC11xx_I2C_MASK2;				//		0x38  -- Slave address mask register 2. This mask
	BT_u32	LPC11xx_I2C_MASK3;				//		0x3C  -- Slave address mask register 3. This mask
} LPC11xx_I2C_REGS;

#define I2C0						((LPC11xx_I2C_REGS *) BT_CONFIG_MACH_LPC11xx_I2C0_BASE)



#endif
