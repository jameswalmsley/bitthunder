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
#define ZYNQ_UART_CR_TXDIS 			0x00000020		///< TX Set Disable bit.
#define ZYNQ_UART_CR_TXEN			0x00000010		///< TX Set Enable bit.
#define ZYNQ_UART_CR_RXDIS			0x00000008		///< RX Set Disable bit.
#define ZYNQ_UART_CR_RXEN 			0x00000004		///< RX Set Enable bit.
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
#define ZYNQ_UART_MR_CLKSEL			0x00000001	///< Pre-scalar selection
#define ZYNQ_UART_MR_CHMODE_L_LOOP	0x00000200	///< Local loop back mode
#define ZYNQ_UART_MR_CHMODE_NORM	0x00000000	///< Normal mode
#define ZYNQ_UART_MR_STOPMODE_2_BIT	0x00000080	///< 2 stop bits
#define ZYNQ_UART_MR_STOPMODE_1_BIT	0x00000000	///< 1 stop bit
#define ZYNQ_UART_MR_PARITY_NONE	0x00000020	///< No parity mode
#define ZYNQ_UART_MR_PARITY_MARK	0x00000018	///< Mark parity mode
#define ZYNQ_UART_MR_PARITY_SPACE	0x00000010	///< Space parity mode
#define ZYNQ_UART_MR_PARITY_ODD		0x00000008	///< Odd parity mode
#define ZYNQ_UART_MR_PARITY_EVEN	0x00000000	///< Even parity mode
#define ZYNQ_UART_MR_CHARLEN_6_BIT	0x00000006	///< 6 bits data
#define ZYNQ_UART_MR_CHARLEN_7_BIT	0x00000004	///< 7 bits data
#define ZYNQ_UART_MR_CHARLEN_8_BIT	0x00000000	///< 8 bits data

	BT_u32	IER;					//      0x08  -- Interrupt Enable Register.
	BT_u32	IDR;					//      0x0C  -- Interrupt Disable Register.
	BT_u32	IMR;					//      0x10  -- Interrupt Mask register.
#define ZYNQ_UART_IXR_TOUT			0x00000100		///< RX Timeout error interrupt
#define ZYNQ_UART_IXR_PARITY		0x00000080		///< Parity error interrupt
#define ZYNQ_UART_IXR_FRAMING		0x00000040		///< Framing error interrupt
#define ZYNQ_UART_IXR_OVERRUN		0x00000020		///< Overrun error interrupt
#define ZYNQ_UART_IXR_TXFULL		0x00000010		///< TX FIFO Full interrupt
#define ZYNQ_UART_IXR_TXEMPTY		0x00000008		///< TX FIFO empty interrupt
#define ZYNQ_UART_IXR_RXTRIG		0x00000001		///< RX FIFO trigger interrupt
#define ZYNQ_UART_IXR_RXFULL		0x00000004		///< RX FIFO full interrupt
#define ZYNQ_UART_IXR_RXEMPTY		0x00000002		///< RX FIFO empty interrupt

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

#endif
