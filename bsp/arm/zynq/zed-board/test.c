#include <bitthunder.h>
#include <string.h>
#include <lib/putc.h>
#include <fs/bt_fs.h>

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

char *buffer = (char *) 0x00100000;

extern void Xil_DCacheFlush();
extern void Xil_ICacheInvalidate();
extern void Xil_DCacheDisable();

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


	BT_ThreadSleep(250);

	BT_TICK time = BT_GetKernelTick();

	BT_HANDLE hVolume = BT_DeviceOpen("mmc00", &Error);

	BT_kPrint("Mounting mmc00");
	BT_Mount(hVolume, "/");
	BT_kPrint("Done");

	BT_ShellCommand("load 0x00100000 /kernel.img");
	BT_ShellCommand("load 0x01000000 /zImage");

	/*BT_kPrint("Opening zImage");

	BT_HANDLE hFile = BT_Open("/kernel.img", "rb", &Error);
	if(!hFile) {
		BT_kPrint("Could not open /kernel.img");
	}

	BT_kPrint("Reading file...");
	BT_Read(hFile, 0, 1024*1024*2, buffer, &Error);
	BT_kPrint("Done");*/

	/*while(1) {
			BT_ThreadSleep(1000);
			BT_ShellCommand("help this is a test");
		}*/

	BT_StopSystemTimer();
	BT_DisableInterrupts();

	Xil_DCacheFlush();
	Xil_ICacheInvalidate();
	//Xil_DCacheDisable();

	BT_CharDeviceFlush(hUART);



	__asm volatile("b 0x00100000");

	while(1) {

		BT_ThreadSleepUntil(&time, 1000);
	}

	return 0;
}
