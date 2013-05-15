#ifndef _SPI_H_
#define _SPI_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LPC11xx_SPI_oDeviceInterface;


typedef struct _LPC11xx_SPI_REGS {
		BT_u32 	CR0;					//		0x00  -- control register 0.

	#define	LPC11xx_SPI_CR0_DSS_MASK				0x0000000F
	#define	LPC11xx_SPI_CR0_FRF_MASK				0x00000030
	#define	LPC11xx_SPI_CR0_CPOL					0x00000040
	#define	LPC11xx_SPI_CR0_CPHA					0x00000080
	#define	LPC11xx_SPI_CR0_SCR_MASK				0x0000FF00

		BT_u32	CR1;					//      0x04  -- control register 1.

	#define	LPC11xx_SPI_CR1_SSP_ENABLE				0x00000002

		BT_u32	DR;						//      0x08  -- FIFO register
		BT_u32	SR;						//		0x0C  -- status register

	#define	LPC11xx_SPI_SR_TFE							0x00000001
	#define	LPC11xx_SPI_SR_TNF							0x00000002
	#define	LPC11xx_SPI_SR_RNE							0x00000004
	#define	LPC11xx_SPI_SR_RFF							0x00000008
	#define	LPC11xx_SPI_SR_BSY							0x00000010

		BT_u32	CPSR;					//      0x10  -- clock prescale register.
		BT_u32	IMSC;					//      0x14  -- interrupt mask register.
		BT_u32	RIS;					//      0x18  -- raw interrupt status register.
		BT_u32	MIS;					//      0x1C  -- masked interrupt status register.
		BT_u32	ICR;					//      0x20  -- interrupt clear register.

} LPC11xx_SPI_REGS;

#define SPI0						((LPC11xx_SPI_REGS *) BT_CONFIG_MACH_LPC11xx_SPI0_BASE)
#define SPI1						((LPC11xx_SPI_REGS *) BT_CONFIG_MACH_LPC11xx_SPI1_BASE)



#endif
