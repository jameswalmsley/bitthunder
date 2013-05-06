#include <bitthunder.h>
#include <string.h>
#include <lib/putc.h>
#include "../../../../arch/arm/mach/lpc11xx/timer.h"


void SetDutyCycle(BT_HANDLE hTimer, BT_u32 ulDutyCycle){
	BT_LPC11xx_TIMER_CONFIG oTimerConfig;

	oTimerConfig.Control			= 0;
	oTimerConfig.ExtMatchControl	= 0;
	oTimerConfig.CaptureControl		= 0;
	oTimerConfig.MatchControl		= 0;
	oTimerConfig.Match[1]			= ulDutyCycle;
	oTimerConfig.PWMControl			= 0x00000002;

	BT_TimerSetConfiguration(hTimer, &oTimerConfig);
}

/**
 *	This will be running within a FreeRTOS Thread, with lowest priority.
 *	It is the boot/startup thread, and you can return from it after starting all your own processes.
 *	Doing so will cause the thread to be killed. Note that any "process owned" resources will also
 *	be automaticallydestroyed.
 *
 *	Do not use this thread to pass resources to other processes.
 *
 *	However, you can just use it as an already running process.
 **/


#define	STEPPER_ENq						37			// Output
#define	STEPPER_DIR						36			// Output
#define	STEPPER_MS2						35			// Output
#define	STEPPER_MS1						34			// Output
#define	STEPPER_RESETq					12			// Output
#define	STEPPER_SLEEPq					22			// Output

#define	STEPPER_STATE_0					24			// Input
#define	STEPPER_STATE_1					39			// Input
#define	STEPPER_STATE_2					30			// Input
#define	TX_SHUTTER_ON					32			// Output
#define	TX_SHUTTER_FB					31			// Input

static void HWInit(void)
{
	// UART Init
	BT_HANDLE hUart = NULL;
	BT_ERROR Error = BT_ERR_NONE;

	hUart = BT_DeviceOpen("uart0", &Error);

	BT_UART_CONFIG oConfig;

	oConfig.eMode		   = BT_UART_MODE_POLLED;
	oConfig.ucDataBits	   = BT_UART_8_DATABITS;
	oConfig.ucStopBits	   = BT_UART_ONE_STOP_BIT;
	oConfig.ucParity	   = BT_UART_PARITY_NONE;
	oConfig.ulBaudrate     = 115200;
	oConfig.ulRxBufferSize = 128;
	oConfig.ulTxBufferSize = 128;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_UartEnable(hUart);

	BT_SetStandardHandle(hUart);
}


int main(int argc, char **argv) {
	BT_ERROR Error = BT_ERR_NONE;

	HWInit();

	BT_HANDLE hCan = BT_DeviceOpen("can0", &Error);

	BT_CAN_CONFIG oCanConfig;

	oCanConfig.eMode			= BT_CAN_MODE_BUFFERED;
	oCanConfig.ulBaudrate		= 1000000;
	oCanConfig.usRxBufferSize	= 8;
	oCanConfig.usTxBufferSize	= 8;

	BT_CanSetConfiguration(hCan, &oCanConfig);
	BT_CanEnable(hCan);

	BT_TICK ticks = BT_kTickCount();

	while(1) {
		BT_CAN_MESSAGE oMessage;

		BT_kTaskDelayUntil(&ticks, 1000);
		BT_CanReadMessage(hCan, &oMessage);
		oMessage.ulID |= 0x100;
		BT_CanSendMessage(hCan, &oMessage);
	}

	return 0;
}

