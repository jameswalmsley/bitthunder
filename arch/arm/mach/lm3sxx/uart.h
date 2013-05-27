#ifndef _UART_H_
#define _UART_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_LM3Sxx_UART_oDeviceInterface;


typedef struct _LM3Sxx_UART_REGS {

	BT_u32 	DR;						//		0x00  -- buffer register.
	union {
		BT_u32	RSR;				//      0x04  -- receive status Register.
		BT_u32	ECR;				//      0x04  -- Error clear Register.
	};
	BT_STRUCT_RESERVED_u32(0, 0x04, 0x18);
	BT_u32	FR;						//      0x18  -- Flag register


#define	LM3Sxx_UART_FR_TXFF				0x00000020
#define LM3Sxx_UART_FR_RXFE				0x00000010  // UART Receive FIFO Empty
#define	LM3Sxx_UART_FR_BUSY				0x00000008

	BT_STRUCT_RESERVED_u32(1, 0x18, 0x20);
	BT_u32	ILPR;					//		0x20  -- IrDA Low Power Register
	BT_u32	IBRD;					//      0x24  -- Integer Baudrate divisor register.
	BT_u32	FBRD;					//      0x28  -- fractional Baudrate divisor register.
	BT_u32	LCRH;					//      0x2C  -- Line control register.

#define LM3Sxx_UART_LCRH_EVEN    		0x00000006  // Even parity
#define LM3Sxx_UART_LCRH_MARK     		0x00000082  // Parity bit is one
#define LM3Sxx_UART_LCRH_SPACE    		0x00000086  // Parity bit is zero
#define LM3Sxx_UART_LCRH_ODD		    0x00000002  // Odd parity
#define	LM3Sxx_UART_LCRH_FEN			0x00000010

	BT_u32	CTL;					//      0x30  -- control register.

#define	LM3Sxx_UART_CTL_RXE				0x00000200
#define	LM3Sxx_UART_CTL_TXE				0x00000100
#define	LM3Sxx_UART_CTL_HSE				0x00000020
#define	LM3Sxx_UART_CTL_UARTEN			0x00000001

	BT_u32	IFLS;					//      0x34  -- interrupt fifo level register.
	BT_u32	IM;						//		0x38  -- interrupt mask register


#define	LM3Sxx_UART_INT_RT				0x00000040
#define	LM3Sxx_UART_INT_TX				0x00000020
#define	LM3Sxx_UART_INT_RX				0x00000010

	BT_u32	RIS;					//      0x3C  -- interrupt raw status register.
	BT_u32	MIS;					//		0x40  -- interrupt masked status register
	BT_u32	ICR;					//		0x44  -- interrupt clear register.
	BT_u32	DMACTL;					//		0x48  -- DMA cotnrol register
	BT_STRUCT_RESERVED_u32(2, 0x48, 0x90);
	BT_u32	LCTL;					//      0x90  -- LIN Control register.
	BT_u32	LSS;					//		0x94  -- LIN snapshot register
	BT_u32	LTIM;					//		0x98  -- LIN Timer register.
	BT_STRUCT_RESERVED_u32(3, 0x98, 0xFD0);
	BT_u32 PeriphID[8];
	BT_u32 CellID[4];
} LM3Sxx_UART_REGS;

#define UART0						((LM3Sxx_UART_REGS *) BT_CONFIG_MACH_LM3Sxx_UART0_BASE)
#define UART1						((LM3Sxx_UART_REGS *) BT_CONFIG_MACH_LM3Sxx_UART1_BASE)
#define UART2						((LM3Sxx_UART_REGS *) BT_CONFIG_MACH_LM3Sxx_UART2_BASE)



#endif
