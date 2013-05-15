#ifndef _SPI_H_
#define _SPI_H_

#include <bitthunder.h>
#include <bt_struct.h>

typedef struct _LPC17xx_SPI_REGS {
	union {
		BT_u32 	CR0;					//		0x00  -- control register 0.
		BT_u32 	CR;						//		0x00  -- control register 0.
	};

#define	LPC17xx_SPI_CR0_DSS_MASK				0x0000000F
#define	LPC17xx_SPI_CR0_FRF_MASK				0x00000030
#define	LPC17xx_SPI_CR0_CPOL					0x00000040
#define	LPC17xx_SPI_CR0_CPHA					0x00000080
#define	LPC17xx_SPI_CR0_SCR_MASK				0x0000FF00

#define	LPC17xx_SPI_CR_DSS_MASK					0x0000000F
#define	LPC17xx_SPI_CR_BIT_ENABLE				0x00000004
#define	LPC17xx_SPI_CR_CPHA						0x00000008
#define	LPC17xx_SPI_CR_CPOL						0x00000010
#define	LPC17xx_SPI_CR_MASTER_MODE				0x00000020
#define	LPC17xx_SPI_CR_LSB_FIRST				0x00000040
#define	LPC17xx_SPI_CR_8BITS					0x00000800


	union {
		BT_u32	CR1;					//      0x04  -- control register 1.
		BT_u32	SPI_SR;
	};

#define	LPC17xx_SPI_SPI_SR_SPIF					0x00000080

#define	LPC17xx_SPI_CR1_SSP_ENABLE				0x00000002

	BT_u32	DR;						//      0x08  -- FIFO register
	union {
		BT_u32	SR;						//		0x0C  -- status register
		BT_u32	CCR;
	};

#define	LPC17xx_SPI_SR_TFE							0x00000001
#define	LPC17xx_SPI_SR_TNF							0x00000002
#define	LPC17xx_SPI_SR_RNE							0x00000004
#define	LPC17xx_SPI_SR_RFF							0x00000008
#define	LPC17xx_SPI_SR_BSY							0x00000010

	BT_u32	CPSR;					//      0x10  -- clock prescale register.
	BT_u32	IMSC;					//      0x14  -- interrupt mask register.
	BT_u32	RIS;					//      0x18  -- raw interrupt status register.
	union {
		BT_u32	MIS;					//      0x1C  -- masked interrupt status register.
		BT_u32	INT;
	};
	BT_u32	ICR;					//      0x20  -- interrupt clear register.

} LPC17xx_SPI_REGS;

#define SPI0						((LPC17xx_SPI_REGS *) BT_CONFIG_MACH_LPC17xx_SPI0_BASE)
#define SPI1						((LPC17xx_SPI_REGS *) BT_CONFIG_MACH_LPC17xx_SPI1_BASE)
#define SPI2						((LPC17xx_SPI_REGS *) BT_CONFIG_MACH_LPC17xx_SPI2_BASE)



#endif
