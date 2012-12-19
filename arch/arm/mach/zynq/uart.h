#ifndef _UART_H_
#define _UART_H_

#include <bitthunder.h>
#include <bt_struct.h>

const BT_IF_DEVICE BT_ZYNQ_UART_oDeviceInterface;


typedef struct _ZYNQ_UART_REGS {
	BT_u32 	CR;						//		0x00  -- Control register.

#define ZYNQ_UART_CR_RESERVED		0xFFFFFE00
#define ZYNQ_UART_CR_STPBRK			0x00000100
#define ZYNQ_UART_CR_STTBRK			0x00000080
#define ZYNQ_UART_CR_RSTTO 			0x00000040
#define ZYNQ_UART_CR_TXDIS 			0x00000020
#define ZYNQ_UART_CR_TXEN			0x00000010
#define ZYNQ_UART_CR_RXDIS			0x00000008
#define ZYNQ_UART_CR_RXEN 			0x00000004
#define ZYNQ_UART_CR_TXRES 			0x00000002
#define ZYNQ_UART_CR_RXRES			0x00000001

#define ZYNQ_UART_CR_STPBRK_VAL(x)	((x & ZYNQ_UART_CR_STPBRK) 	>> 8)
#define ZYNQ_UART_CR_STTBRK_VAL(x)	((x & ZYNQ_UART_CR_STTBRK) 	>> 7)
#define ZYNQ_UART_CR_RSTTO_VAL(x)	((x & ZYNQ_UART_CR_RSTTO) 	>> 6)
#define ZYNQ_UART_CR_TXDIS_VAL(x)	((x & ZYNQ_UART_CR_TXDIS) 	>> 5)
#define ZYNQ_UART_CR_TXEN_VAL(x)	((x & ZYNQ_UART_CR_TXEN) 	>> 4)
#define ZYNQ_UART_CR_RXDIS_VAL(x)	((x & ZYNQ_UART_CR_RXDIS) 	>> 3)
#define ZYNQ_UART_CR_RXEN_VAL(x)	((x & ZYNQ_UART_CR_RXEN) 	>> 2)
#define ZYNQ_UART_CR_TXRES_VAL(x)	((x & ZYNQ_UART_CR_TXRES)	>> 1)
#define ZYNQ_UART_CR_RXRES_VAL(x)	((x & ZYNQ_UART_CR_RXRES) 	>> 0)

	BT_u32	MR;						//		0x04  -- Mode Register
	BT_u32	IER;					//      0x08  -- Interrupt Enable Register.
	BT_u32	IDR;					//      0x0C  -- Interrupt Disable Register.
	BT_u32	IMR;					//      0x10  -- Interrupt Mask register.
	BT_u32	ISR;					//      0x14  -- Interrupt Status register.
	BT_u32	BAUDGEN;				//      0x18  -- Baud rate generator.
	BT_u32	RXTOUT;					//      0x1C  -- RX timeout register.
	BT_u32	RXTRIG;					//      0x20  -- RX fifo trigger level.
	BT_u32	MODEMCR;				//      0x24  -- Modem control register.
	BT_u32	MODEMSR;				//      0x28  -- Modem status register.
	BT_u32	SR;						//      0x2C  -- Channel status register.

#define ZYNQ_UART_SR_RESERVED		0xFFFF8000
#define ZYNQ_UART_SR_TXNFULL		0x00004000
#define ZYNQ_UART_SR_TXTRIG			0x00002000
#define ZYNQ_UART_SR_FDELTRIG		0x00001000
#define ZYNQ_UART_SR_TXACTIVE		0x00000800
#define ZYNQ_UART_SR_RXACTIVE 		0x00000400
#define ZYNQ_UART_SR_DMSI 			0x00000200
#define ZYNQ_UART_SR_TIMEOUT 		0x00000100
#define ZYNQ_UART_SR_PARITY			0x00000080
#define ZYNQ_UART_SR_FRAME 			0x00000040
#define ZYNQ_UART_SR_RXOVR 			0x00000020
#define ZYNQ_UART_SR_TXFULL 		0x00000010
#define ZYNQ_UART_SR_TXEMPTY		0x00000008
#define ZYNQ_UART_SR_RXFULL 		0x00000004
#define ZYNQ_UART_SR_RXEMPTY		0x00000002
#define ZYNQ_UART_SR_RTRIG 			0x00000001

#define ZYNQ_UART_SR_TNFUL_VAL(x)	((x & ZYNQ_UART_SR_TNFUL) >> 14)

#define ZYNQ_UART_SR_TTRIG_VAL(x)	((x & ZYNQ_UART_SR_TTRIG) >> 13)

#define ZYNQ_UART_SR_FDELT_VAL(x)	((x & ZYNQ_UART_SR_FDELT_VAL) >> 12)


	BT_u32	FIFO;       			//      0x30  -- TX_RX fifo.
	BT_u32	BAUDDIV;    			//      0x34  -- Baudrate divider.
	BT_u32	FLOWDEL;    			//      0x38  -- Flow delay register

	BT_STRUCT_RESERVED_u32(0, 0x38, 0x44);

	BT_u32	TXTRIG;       			//      0x44  -- TX fifo level trigger register.
} ZYNQ_UART_REGS;

#define UART0						((ZYNQ_UART_REGS *) 0xE0000000)
#define UART1						((ZYNQ_UART_REGS *) 0xE0001000)

#endif
