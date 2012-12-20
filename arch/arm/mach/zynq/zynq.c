#include <bitthunder.h>
#include "timer.h"
#include "uart.h"
#include <arch/common/gic.h>








BT_MACHINE_START(ARM, ZYNQ, "Xilinx Embedded Zynq Platform")
    .ulSystemClockHz 		= BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ,
	.pInterruptController	= &BT_ARM_GIC_oInterface,
	.ulTotalIRQs			= 95,
	.pSystemTimer 			= &BT_ZYNQ_TIMER_oDeviceInterface,
	.ulTimerID				= BT_CONFIG_MACH_ZYNQ_SYSTICK_TIMER_ID,
	.pBootUart				= &BT_ZYNQ_UART_oDeviceInterface,
	.ulBootUartID			= BT_CONFIG_MACH_ZYNQ_BOOT_UART_ID,
BT_MACHINE_END
