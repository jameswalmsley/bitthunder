#ifndef _I2C_H_
#define _I2C_H_

#include <bitthunder.h>

typedef struct _ZYNQ_I2C_REGS {
	BT_u32	CONTROL;
    #define CONTROL_DIV_A				0x0000C000		///< Divisor for Stage A clock divider.
    #define CONTROL_DIV_A_GET(x)		((x & CONTROL_DIV_A) >> 14)
    #define CONTROL_DIV_A_SET(x, val)	(x = ((x & ~CONTROL_DIV_A) | ((val & 0x3) << 14)))
    #define CONTROL_DIV_B				0x00003F00		///< Divisor for Stage B clock divider.
    #define CONTROL_DIV_B_GET(x)		((x & CONTROL_DIV_B) >> 8)
    #define CONTROL_DIV_B_SET(x, val)	(x = ((x & ~CONTROL_DIV_B) | ((val & 0x3F) << 8)))
    #define CONTROL_CLR_FIFO			0x00000040		///< Initialises the fifo to all zeros, and clears the transfer size reg.
    #define CONTROL_SLVMON				0x00000020		///< Slave monitor mode.
    #define CONTROL_HOLD				0x00000010 		///< Bus hold (Clock stretching).
    #define CONTROL_ACK_EN				0x00000008 		///< 1, means send ACK, 0 sends NACK.
	#define CONTROL_NEA					0x00000004		///< Interface addressing mode (1 = 7bit).
    #define CONTROL_MS					0x00000002		///< Interface is master.
	#define CONTROL_RW					0x00000001		///< Master Transfer direction (0 tx, 1 rx).

	BT_u32	STATUS;
    #define STATUS_BA					0x00000100		///< Bus active.
    #define STATUS_RXOVF				0x00000080		///< Receiver overflow.
    #define STATUS_TXDV					0x00000040		///< Transmit data valid.
    #define STATUS_RXDV					0x00000020 		///< Receiver data valid.
    #define STATUS_RXRW					0x00000008 		///< Mode of transmission received from a master.

	BT_u32	ADDRESS;
    #define ADDRESS_ADD					0x000003FF 		///< I2C address.

	BT_u32	DATA;
    #define DATA_DATA					0x000000FF		///< Data to be transmitted / that which was received.

	BT_u32	INT_STATUS;
    #define INT_STATUS_ARB_LOST			0x00000200		///< W2C Arbitration lost.
    #define INT_STATUS_RX_UNF			0x00000080 		///< W2C Fifo receive underflow.
    #define INT_STATUS_TX_OVF			0x00000040 		///< W2C Tx overflow.
    #define INT_STATUS_RX_OVF			0x00000020 		///< W2C Rx overflow.
    #define INT_STATUS_SLV_RDY			0x00000010 		///< W2C Monitored slave ready.
    #define INT_STATUS_TO				0x00000008 		///< W2C Transfer time out.
    #define INT_STATUS_NACK				0x00000004 		///< W2C Transfer not acknowledged.
    #define INT_STATUS_DATA				0x00000002 		///< W2C More data.
    #define INT_STATUS_COMP				0x00000001 		///< W2C Transfer complete.

	BT_u32	TRANSFER_SIZE;
    #define TRANSFER_SIZE_SIZE			0x000000FF		///< Transfer size.

	BT_u32	SLAVE_PAUSE;
    #define SLAVE_PAUSE_PAUSE			0x0000000F 		///< Pause interval (0 - 7).

	BT_u32	TIMEOUT;
    #define TIMEOUT_TO					0x000000FF		///< Timeout register.

	BT_u32	INT_MASK;
	#define INT_MASK_ARB_LOST			0x00000200		///< Arbitration lost.
	#define INT_MASK_RX_UNF				0x00000080		///< Fifo receive underflow.
	#define INT_MASK_TX_OVF				0x00000040		///< Tx overflow.
	#define INT_MASK_RX_OVF				0x00000020		///< Rx overflow.
	#define INT_MASK_SLV_RDY			0x00000010		///< Monitored slave ready.
	#define INT_MASK_TO					0x00000008		///< Transfer time out.
	#define INT_MASK_NACK				0x00000004		///< Transfer not acknowledged.
	#define INT_MASK_DATA				0x00000002		///< More data.
	#define INT_MASK_COMP				0x00000001		///< Transfer complete.

	BT_u32	INT_ENABLE;
	#define INT_ENABLE_ARB_LOST			0x00000200		///< Arbitration lost.
	#define INT_ENABLE_RX_UNF			0x00000080		///< Fifo receive underflow.
	#define INT_ENABLE_TX_OVF			0x00000040		///< Tx overflow.
	#define INT_ENABLE_RX_OVF			0x00000020		///< Rx overflow.
	#define INT_ENABLE_SLV_RDY			0x00000010		///< Monitored slave ready.
	#define INT_ENABLE_TO				0x00000008		///< Transfer time out.
	#define INT_ENABLE_NACK				0x00000004		///< Transfer not acknowledged.
	#define INT_ENABLE_DATA				0x00000002		///< More data.
	#define INT_ENABLE_COMP				0x00000001		///< Transfer complete.

	BT_u32	INT_DISABLE;
	#define INT_DISABLE_ARB_LOST		0x00000200		///< Arbitration lost.
	#define INT_DISABLE_RX_UNF			0x00000080		///< Fifo receive underflow.
	#define INT_DISABLE_TX_OVF			0x00000040		///< Tx overflow.
	#define INT_DISABLE_RX_OVF			0x00000020		///< Rx overflow.
	#define INT_DISABLE_SLV_RDY			0x00000010		///< Monitored slave ready.
	#define INT_DISABLE_TO				0x00000008		///< Transfer time out.
	#define INT_DISABLE_NACK			0x00000004		///< Transfer not acknowledged.
	#define INT_DISABLE_DATA			0x00000002		///< More data.
	#define INT_DISABLE_COMP			0x00000001		///< Transfer complete.

} ZYNQ_I2C_REGS;

#define I2C_FIFO_LEN	16









#endif
