#include <bitthunder.h>
#include <string.h>
#include <lib/putc.h>
#include <fs/bt_fs.h>

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

char *buffer = 0x00100000;

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

	BT_SetStandardHandle(hUART);

	BT_GpioSetDirection(7, BT_GPIO_DIR_OUTPUT);

	bt_printf("\r\n");

	BT_ThreadSleep(2000);

	BT_TICK time = BT_GetKernelTick();

	BT_HANDLE hVolume = BT_DeviceOpen("mmc00", &Error);

	BT_Mount(hVolume, "/");


	BT_kPrint("Opening zImage");

	BT_HANDLE hFile = BT_Open("/zImage", "rb", &Error);

	BT_kPrint("Reading file...");
	BT_Read(hFile, 0, 1024*1024*2, buffer, &Error);
	BT_kPrint("Done");

	while(1) {
		BT_u32 i;
		BT_u32 ticks = BT_GetKernelTime();

		BT_u32 offset = BT_GetSystemTimerOffset();
		//bt_printf("Time: %d.%03d\r", ticks/1000000, ticks%1000000);
		BT_ThreadSleepUntil(&time, 1000);


	}

	return 0;
}
