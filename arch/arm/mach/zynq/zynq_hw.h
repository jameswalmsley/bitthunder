/*
 * zynq_hw.h
 *
 *  Created on: Oct 4, 2012
 *      Author: root
 */

#ifndef ZYNQ_HW_H_
#define ZYNQ_HW_H_

#ifdef __cplusplus
extern "C" {
#endif



// Peripheral Base Addresses
#define ZYNQ_UART0_BASE	 			0xE0000000
#define ZYNQ_UART1_BASE				0xE0001000
#define ZYNQ_INTC_ICC_BASE			0xF8F00100
#define ZYNQ_INTC_ICD_BASE			0xF8F01000


#define ZYNQ_UART_CR				0x00  /**< Control Register [8:0] */
#define ZYNQ_UART_MR				0x04  /**< Mode Register [10:0] */
#define ZYNQ_UART_IER				0x08  /**< Interrupt Enable [10:0] */
#define ZYNQ_UART_IDR_OFFSET		0x0C  /**< Interrupt Disable [10:0] */
#define ZYNQ_UART_IMR_OFFSET		0x10  /**< Interrupt Mask [10:0] */
#define ZYNQ_UART_ISR_OFFSET		0x14  /**< Interrupt Status [10:0]*/
#define ZYNQ_UART_BAUDGEN_OFFSET	0x18  /**< Baud Rate Generator [15:0] */
#define ZYNQ_UART_RXTOUT_OFFSET		0x1C  /**< RX Timeout [7:0] */
#define ZYNQ_UART_RXWM_OFFSET		0x20  /**< RX FIFO Trigger Level [5:0] */
#define ZYNQ_UART_MODEMCR_OFFSET	0x24  /**< Modem Control [5:0] */
#define ZYNQ_UART_MODEMSR_OFFSET	0x28  /**< Modem Status [8:0] */
#define ZYNQ_UART_SR_OFFSET			0x2C  /**< Channel Status [11:0] */
#define ZYNQ_UART_FIFO_OFFSET		0x30  /**< FIFO [15:0] or [7:0] */
#define ZYNQ_UART_BAUDDIV_OFFSET	0x34  /**< Baud Rate Divider [7:0] */
#define ZYNQ_UART_FLOWDEL_OFFSET	0x38  /**< Flow Delay [15:0] */
#define ZYNQ_UART_TXWM_OFFSET		0x44  /**< TX FIFO Trigger Level [5:0] */
/* @} */

/** @name Control Register
 *
 * The Control register (CR) controls the major functions of the device.
 *
 * Control Register Bit Definition
 */

#include "ps7_init.h"

#define XUARTPS_CR_STOPBRK			0x00000100  /**< Stop transmission of break */
#define XUARTPS_CR_STARTBRK			0x00000080  /**< Set break */
#define XUARTPS_CR_TORST			0x00000040  /**< RX timeout counter restart */
#define XUARTPS_CR_TX_DIS			0x00000020  /**< TX disabled. */
#define XUARTPS_CR_TX_EN			0x00000010  /**< TX enabled */
#define XUARTPS_CR_RX_DIS			0x00000008  /**< RX disabled. */
#define XUARTPS_CR_RX_EN			0x00000004  /**< RX enabled */
#define XUARTPS_CR_EN_DIS_MASK		0x0000003C  /**< Enable/disable Mask */
#define XUARTPS_CR_TXRST			0x00000002  /**< TX logic reset */
#define XUARTPS_CR_RXRST			0x00000001  /**< RX logic reset */
/* @}*/


/** @name Mode Register
 *
 * The mode register (MR) defines the mode of transfer as well as the data
 * format. If this register is modified during transmission or reception,
 * data validity cannot be guaranteed.
 *
 * Mode Register Bit Definition
 * @{
 */

#define XUARTPS_MR_CCLK				0x00000400 /**< Input clock selection */
#define XUARTPS_MR_CHMODE_R_LOOP	0x00000300 /**< Remote loopback mode */
#define XUARTPS_MR_CHMODE_L_LOOP	0x00000200 /**< Local loopback mode */
#define XUARTPS_MR_CHMODE_ECHO		0x00000100 /**< Auto echo mode */
#define XUARTPS_MR_CHMODE_NORM		0x00000000 /**< Normal mode */
#define XUARTPS_MR_CHMODE_SHIFT			8  /**< Mode shift */
#define XUARTPS_MR_CHMODE_MASK		0x00000300 /**< Mode mask */
#define XUARTPS_MR_STOPMODE_2_BIT	0x00000080 /**< 2 stop bits */
#define XUARTPS_MR_STOPMODE_1_5_BIT	0x00000040 /**< 1.5 stop bits */
#define XUARTPS_MR_STOPMODE_1_BIT	0x00000000 /**< 1 stop bit */
#define XUARTPS_MR_STOPMODE_SHIFT		6  /**< Stop bits shift */
#define XUARTPS_MR_STOPMODE_MASK	0x000000A0 /**< Stop bits mask */
#define XUARTPS_MR_PARITY_NONE		0x00000020 /**< No parity mode */
#define XUARTPS_MR_PARITY_MARK		0x00000018 /**< Mark parity mode */
#define XUARTPS_MR_PARITY_SPACE		0x00000010 /**< Space parity mode */
#define XUARTPS_MR_PARITY_ODD		0x00000008 /**< Odd parity mode */
#define XUARTPS_MR_PARITY_EVEN		0x00000000 /**< Even parity mode */
#define XUARTPS_MR_PARITY_SHIFT		3  /**< Parity setting shift */
#define XUARTPS_MR_PARITY_MASK		0x00000038 /**< Parity mask */
#define XUARTPS_MR_CHARLEN_6_BIT	0x00000006 /**< 6 bits data */
#define XUARTPS_MR_CHARLEN_7_BIT	0x00000004 /**< 7 bits data */
#define XUARTPS_MR_CHARLEN_8_BIT	0x00000000 /**< 8 bits data */
#define XUARTPS_MR_CHARLEN_SHIFT		1  /**< Data Length shift */
#define XUARTPS_MR_CHARLEN_MASK		0x00000006 /**< Data length mask */
#define XUARTPS_MR_CLKSEL		0x00000001 /**< Input clock selection */
/* @} */


/** @name Interrupt Registers
 *
 * Interrupt control logic uses the interrupt enable register (IER) and the
 * interrupt disable register (IDR) to set the value of the bits in the
 * interrupt mask register (IMR). The IMR determines whether to pass an
 * interrupt to the interrupt status register (ISR).
 * Writing a 1 to IER Enbables an interrupt, writing a 1 to IDR disables an
 * interrupt. IMR and ISR are read only, and IER and IDR are write only.
 * Reading either IER or IDR returns 0x00.
 *
 * All four registers have the same bit definitions.
 *
 * @{
 */

#define XUARTPS_IXR_DMS		0x00000200 /**< Modem status change interrupt */
#define XUARTPS_IXR_TOUT	0x00000100 /**< Timeout error interrupt */
#define XUARTPS_IXR_PARITY 	0x00000080 /**< Parity error interrupt */
#define XUARTPS_IXR_FRAMING	0x00000040 /**< Framing error interrupt */
#define XUARTPS_IXR_OVER	0x00000020 /**< Overrun error interrupt */
#define XUARTPS_IXR_TXFULL 	0x00000010 /**< TX FIFO full interrupt. */
#define XUARTPS_IXR_TXEMPTY	0x00000008 /**< TX FIFO empty interrupt. */
#define XUARTPS_IXR_RXFULL 	0x00000004 /**< RX FIFO full interrupt. */
#define XUARTPS_IXR_RXEMPTY	0x00000002 /**< RX FIFO empty interrupt. */
#define XUARTPS_IXR_RXOVR  	0x00000001 /**< RX FIFO trigger interrupt. */
#define XUARTPS_IXR_MASK	0x000003FF /**< Valid bit mask */
/* @} */


/** @name Baud Rate Generator Register
 *
 * The baud rate generator control register (BRGR) is a 16 bit register that
 * controls the receiver bit sample clock and baud rate.
 * Valid values are 1 - 65535.
 *
 * Bit Sample Rate = CCLK / BRGR, where the CCLK is selected by the MR_CCLK bit
 * in the MR register.
 * @{
 */
#define XUARTPS_BAUDGEN_DISABLE		0x00000000 /**< Disable clock */
#define XUARTPS_BAUDGEN_MASK		0x0000FFFF /**< Valid bits mask */

/** @name Baud Divisor Rate register
 *
 * The baud rate divider register (BDIV) controls how much the bit sample
 * rate is divided by. It sets the baud rate.
 * Valid values are 0x04 to 0xFF. Writing a value less than 4 will be ignored.
 *
 * Baud rate = CCLK / ((BAUDDIV + 1) x BRGR), where the CCLK is selected by
 * the MR_CCLK bit in the MR register.
 * @{
 */
#define XUARTPS_BAUDDIV_MASK  0x000000FF	/**< 8 bit baud divider mask */
/* @} */


/** @name Receiver Timeout Register
 *
 * Use the receiver timeout register (RTR) to detect an idle condition on
 * the receiver data line.
 *
 * @{
 */
#define XUARTPS_RXTOUT_DISABLE		0x00000000  /**< Disable time out */
#define XUARTPS_RXTOUT_MASK		0x000000FF  /**< Valid bits mask */

/** @name Receiver FIFO Trigger Level Register
 *
 * Use the Receiver FIFO Trigger Level Register (RTRIG) to set the value at
 * which the RX FIFO triggers an interrupt event.
 * @{
 */

#define XUARTPS_RXWM_DISABLE	0x00000000  /**< Disable RX trigger interrupt */
#define XUARTPS_RXWM_MASK	0x0000003F  /**< Valid bits mask */
/* @} */

/** @name Modem Control Register
 *
 * This register (MODEMCR) controls the interface with the modem or data set,
 * or a peripheral device emulating a modem.
 *
 * @{
 */
#define XUARTPS_MODEMCR_FCM	0x00000010  /**< Flow control mode */
#define XUARTPS_MODEMCR_RTS	0x00000002  /**< Request to send */
#define XUARTPS_MODEMCR_DTR	0x00000001  /**< Data terminal ready */
/* @} */

/** @name Modem Status Register
 *
 * This register (MODEMSR) indicates the current state of the control lines
 * from a modem, or another peripheral device, to the CPU. In addition, four
 * bits of the modem status register provide change information. These bits
 * are set to a logic 1 whenever a control input from the modem changes state.
 *
 * Note: Whenever the DCTS, DDSR, TERI, or DDCD bit is set to logic 1, a modem
 * status interrupt is generated and this is reflected in the modem status
 * register.
 *
 * @{
 */
#define XUARTPS_MODEMSR_FCMS	0x00000100  /**< Flow control mode (FCMS) */
#define XUARTPS_MODEMSR_DCD	0x00000080  /**< Complement of DCD input */
#define XUARTPS_MODEMSR_RI	0x00000040  /**< Complement of RI input */
#define XUARTPS_MODEMSR_DSR	0x00000020  /**< Complement of DSR input */
#define XUARTPS_MODEMSR_CTS	0x00000010  /**< Complement of CTS input */
#define XUARTPS_MEDEMSR_DCDX	0x00000008  /**< Delta DCD indicator */
#define XUARTPS_MEDEMSR_RIX	0x00000004  /**< Change of RI */
#define XUARTPS_MEDEMSR_DSRX	0x00000002  /**< Change of DSR */
#define XUARTPS_MEDEMSR_CTSX	0x00000001  /**< Change of CTS */
/* @} */

/** @name Channel Status Register
 *
 * The channel status register (CSR) is provided to enable the control logic
 * to monitor the status of bits in the channel interrupt status register,
 * even if these are masked out by the interrupt mask register.
 *
 * @{
 */

#define XUARTPS_SR_FLOWDEL	0x00001000 /**< RX FIFO fill over flow delay */
#define XUARTPS_SR_TACTIVE	0x00000800 /**< TX active */
#define XUARTPS_SR_RACTIVE	0x00000400 /**< RX active */
#define XUARTPS_SR_DMS		0x00000200 /**< Delta modem status change */
#define XUARTPS_SR_TOUT		0x00000100 /**< RX timeout */
#define XUARTPS_SR_PARITY	0x00000080 /**< RX parity error */
#define XUARTPS_SR_FRAME	0x00000040 /**< RX frame error */
#define XUARTPS_SR_OVER		0x00000020 /**< RX overflow error */
#define XUARTPS_SR_TXFULL	0x00000010 /**< TX FIFO full */
#define XUARTPS_SR_TXEMPTY	0x00000008 /**< TX FIFO empty */
#define XUARTPS_SR_RXFULL	0x00000004 /**< RX FIFO full */
#define XUARTPS_SR_RXEMPTY	0x00000002 /**< RX FIFO empty */
#define XUARTPS_SR_RXOVR	0x00000001 /**< RX FIFO fill over trigger */
/* @} */

/** @name Flow Delay Register
 *
 * Operation of the flow delay register (FLOWDEL) is very similar to the
 * receive FIFO trigger register. An internal trigger signal activates when the
 * FIFO is filled to the level set by this register. This trigger will not
 * cause an interrupt, although it can be read through the channel status
 * register. In hardware flow control mode, RTS is deactivated when the trigger
 * becomes active. RTS only resets when the FIFO level is four less than the
 * level of the flow delay trigger and the flow delay trigger is not activated.
 * A value less than 4 disables the flow delay.
 * @{
 */
#define XUARTPS_FLOWDEL_MASK	XUARTPS_RXWM_MASK	/**< Valid bit mask */
/* @} */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
* Read a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*		device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XUartPs_ReadReg(u32 BaseAddress, int RegOffset)
*
******************************************************************************/
#define XUartPs_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*		device.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUartPs_WriteReg(u32 BaseAddress, int RegOffset,
*						   u16 RegisterValue)
*
******************************************************************************/
#define XUartPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/****************************************************************************/
/**
* Determine if there is receive data in the receiver and/or FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if there is receive data, FALSE otherwise.
*
* @note		C-Style signature:
*		u32 XUartPs_IsReceiveData(u32 BaseAddress)
*
******************************************************************************/
#define XUartPs_IsReceiveData(BaseAddress)			 \
	!((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	XUARTPS_SR_RXEMPTY) == XUARTPS_SR_RXEMPTY)

/****************************************************************************/
/**
* Determine if a byte of data can be sent with the transmitter.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if the TX FIFO is full, FALSE if a byte can be put in the
*		FIFO.
*
* @note		C-Style signature:
*		u32 XUartPs_IsTransmitFull(u32 BaseAddress)
*
******************************************************************************/
#define XUartPs_IsTransmitFull(BaseAddress)			 \
	((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	 XUARTPS_SR_TXFULL) == XUARTPS_SR_TXFULL)



#ifdef __cplusplus
}
#endif



#endif /* ZYNQ_HW_H_ */
