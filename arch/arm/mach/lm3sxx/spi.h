#ifndef _SPI_H_
#define _SPI_H_

#include <bitthunder.h>
#include <bt_struct.h>

typedef struct _LM3Sxx_SPI_REGS {
	BT_u32 	CR0;					//		0x00  -- control register 0.

#define	LM3Sxx_SPI_CR0_DSS_MASK				0x0000000F
#define	LM3Sxx_SPI_CR0_FRF_MASK				0x00000030
#define	LM3Sxx_SPI_CR0_CPOL					0x00000040
#define	LM3Sxx_SPI_CR0_CPHA					0x00000080
#define	LM3Sxx_SPI_CR0_SCR_MASK				0x0000FF00

	BT_u32	CR1;					//      0x04  -- control register 1.

#define	LM3Sxx_SPI_SPI_SR_SPIF					0x00000080
#define	LM3Sxx_SPI_CR1_SSP_ENABLE				0x00000002

	BT_u32	DR;						//      0x08  -- FIFO register
	BT_u32	SR;						//		0x0C  -- status register

#define	LM3Sxx_SPI_SR_TFE							0x00000001
#define	LM3Sxx_SPI_SR_TNF							0x00000002
#define	LM3Sxx_SPI_SR_RNE							0x00000004
#define	LM3Sxx_SPI_SR_RFF							0x00000008
#define	LM3Sxx_SPI_SR_BSY							0x00000010

	BT_u32	CPSR;					//      0x10  -- clock prescale register.
	BT_u32	IM;						//      0x14  -- interrupt mask register.
	BT_u32	RIS;					//      0x18  -- raw interrupt status register.
	BT_u32	MIS;					//      0x1C  -- masked interrupt status register.
	BT_u32	ICR;					//      0x20  -- interrupt clear register.
	BT_u32	DMACTL;					//      0x24  -- DMA control register.
	BT_STRUCT_RESERVED_u32(0, 0x24, 0xFD0);
	BT_u32 PeriphID[8];
	BT_u32 CellID[4];
} LM3Sxx_SPI_REGS;

#define SPI0						((LM3Sxx_SPI_REGS *) BT_CONFIG_MACH_LM3Sxx_SPI0_BASE)
#define SPI1						((LM3Sxx_SPI_REGS *) BT_CONFIG_MACH_LM3Sxx_SPI1_BASE)



#endif
