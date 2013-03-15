#include <bitthunder.h>
#include <string.h>

extern BT_u32 BT_ZYNQ_GetArmPLLFrequency();

/**
 *	This will be running within a FreeRTOS Thread, with lowest priority.
 *	It is the boot/startup thread, and you can return from it after starting all your own processes.
 *	Doing so will cause the thread to be killed. Note that any "process owned" resources will also
 *	be automatically destroyed.
 *
 *	Do not use this thread to pass resources to other processes.
 *
 *	However, you can just use it as an already running process.
 **/

int main(int argc, char **argv) {

	BT_ERROR Error;
	BT_HANDLE hUART = BT_DeviceOpen("uart1", &Error);

	BT_UART_CONFIG oConfig;
	//BT_SetPowerState(hUart, BT_POWER_STATE_AWAKE);

	oConfig.eMode 		= BT_UART_MODE_POLLED;
	oConfig.ucDataBits 	= 8;
	oConfig.ulBaudrate 	= 115200;

	BT_UartSetConfiguration(hUART, &oConfig);

	BT_UartEnable(hUART);

	BT_GpioSetDirection(7, BT_GPIO_DIR_OUTPUT);

	BT_u32 i = 0;

	while(1) {
		BT_CharDeviceWrite(hUART, 0, 1, (BT_u8 *) ".");
		BT_ThreadSleep(1000);
		i++;
		if(i % 10 == 0) {
			BT_CharDeviceWrite(hUART, 0, 1, (BT_u8 *) "\n");
		}
	}

	return 0;
}
