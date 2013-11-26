#ifndef _UART_H_
#define _UART_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LPC11xx_UART_oDeviceInterface;


typedef struct _LPC11xx_UART_REGS {
	union {
		BT_u32 	FIFO;					//		0x00  -- buffer register.
		BT_u32	DLL;					//		0x00  -- LSB of Baudrate divider
	};
	union {
		BT_u32	IER;					//      0x04  -- Interrupt Enable Register.
		BT_u32	DLM;					//		0x04  -- MSB of Baudrate divider
	};

#define LPC11xx_UART_IER_THREIE			0x00000002
#define LPC11xx_UART_IER_RBRIE			0x00000001

	union {
		BT_u32	FCR;					//      0x08  -- FIFO control register
		BT_u32	IIR;					//		0x08  -- Interrupt Identification Register.
	};

#define	LPC11xx_UART_FCR_RX_LEVEL14		0x000000C0
#define	LPC11xx_UART_FCR_RX_LEVEL8		0x00000080
#define	LPC11xx_UART_FCR_RX_LEVEL4		0x00000040
#define	LPC11xx_UART_FCR_RX_LEVEL1		0x00000000
#define	LPC11xx_UART_FCR_TX_PURGE		0x00000004
#define	LPC11xx_UART_FCR_RX_PURGE		0x00000002
#define	LPC11xx_UART_FCR_FIFO_ENB		0x00000001

#define	LPC11xx_UART_IIR_THRE_INT		0x00000002
#define	LPC11xx_UART_IIR_RDA_INT		0x00000004
#define	LPC11xx_UART_IIR_RLS_INT		0x0000000C

	BT_u32	LCR;					//		0x0C  -- Line Control Register

#define	LPC11xx_UART_LCR_DLAB			0x00000080
#define	LPC11xx_UART_LCR_BRK			0x00000040
#define	LPC11xx_UART_LCR_PS_ZERO		0x00000030
#define	LPC11xx_UART_LCR_PS_ONE			0x00000020
#define	LPC11xx_UART_LCR_PS_EVEN		0x00000010
#define	LPC11xx_UART_LCR_PS_ODD			0x00000000
#define	LPC11xx_UART_LCR_PE				0x00000008
#define	LPC11xx_UART_LCR_SBS_2			0x00000004
#define	LPC11xx_UART_LCR_SBS_1			0x00000000
#define	LPC11xx_UART_LCR_WLS_8			0x00000003
#define	LPC11xx_UART_LCR_WLS_7			0x00000002
#define	LPC11xx_UART_LCR_WLS_6			0x00000001
#define	LPC11xx_UART_LCR_WLS_5			0x00000000

	BT_u32	MCR;					//      0x10  -- Modem Control register.
	BT_u32	LSR;					//      0x14  -- Line Status register.

#define	LPC11xx_UART_LSR_RXFE			0x00000080
#define	LPC11xx_UART_LSR_TEMT			0x00000040
#define	LPC11xx_UART_LSR_THRE			0x00000020
#define	LPC11xx_UART_LSR_BI				0x00000010
#define	LPC11xx_UART_LSR_FE				0x00000008
#define	LPC11xx_UART_LSR_PE				0x00000004
#define	LPC11xx_UART_LSR_OE				0x00000002
#define	LPC11xx_UART_LSR_RDR			0x00000001

	BT_u32	MSR;					//      0x18  -- Modem Status register.
	BT_u32	SCR;					//      0x1C  -- Scratch Pad register.
	BT_u32	ABCR;					//      0x20  -- Auto-baud control register.
	BT_u32	ICR;					//		0x24  -- IrDA Control register
	BT_u32	FDR;					//      0x28  -- Fractional Divider register.
	BT_u32	OSR;					//		0x2C  -- Oversampling register
	BT_u32	TER;					//		0x30  -- Transmit Enable register.

#define	LPC11xx_UART_TER_TXEN			0x00000080

	BT_STRUCT_RESERVED_u32(2, 0x30, 0x48);
	BT_u32	SCICTRL;				//		0x48  -- Smart Card Interface Control register
	BT_u32	RS485CR;				//		0x4C  -- RS485 control register.
	BT_u32	RS485AMR;				//		0x50  -- RS485 address match register.
	BT_u32	RS485DLY;				//		0x54  -- RS485 Delay value register.
	BT_u32	SYNCCTRL;				//		0x58  -  Sync mode control register

} LPC11xx_UART_REGS;

#define UART0						((LPC11xx_UART_REGS *) BT_CONFIG_MACH_LPC11xx_UART0_BASE)


#endif
