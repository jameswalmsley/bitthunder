/**
 *	This is the operating system main entry point, from where it will boot and load
 *	kernel modules.
 *
 *	Once all kernel modules are loaded and initialised, we shall look user application
 *	processes to be loaded.
 *
 **/

#include <bitthunder.h>
#include <mm/bt_heap.h>
#include <string.h>

extern int main(int argc, char **argv);

/**
 *	@ Note: argc and argv may be used for kernel booting parameters at a later date.
 *
 *
 **/
int bt_main(int argc, char **argv) {

	BT_ERROR Error;

	// Get Machine Description.
	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(pMachine->szpName) {
		Error = 0;
	}

	BT_HANDLE hUart = pMachine->pBootUart->pfnOpen(pMachine->ulBootUartID, &Error);
	if(!hUart) {
		// Well, we're completely out of luck! We cannot report this to anything.
		return -1;
	}

	BT_UART_CONFIG oConfig;
	//BT_SetPowerState(hUart, BT_POWER_STATE_AWAKE);

	oConfig.eMode 		= BT_UART_MODE_POLLED;
	oConfig.ucDataBits 	= 8;
	oConfig.ulBaudrate 	= 115200;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_UartEnable(hUart);

	char *string = "BitThunder v0.0.1\r\n";
	BT_CharDeviceWrite(hUart, 0, strlen(string), (BT_u8 *)string);

	string = "Start Loading kernel modules...\r\n";
	BT_CharDeviceWrite(hUart, 0, strlen(string), (BT_u8 *)string);

	string = "Initialise interrupt controller...\r\n";
	BT_CharDeviceWrite(hUart, 0, strlen(string), (BT_u8 *)string);

	Error = pMachine->pInterruptController->pfnInitialise(pMachine->ulTotalIRQs);

	// Start Scheduler
	string = "Enter user-mode, and start user-space application...\r\n";
	BT_CharDeviceWrite(hUart, 0, strlen(string), (BT_u8 *)string);

	string = "Relinquish control of the boot UART device...(Goodbye)\r\n";
	BT_CharDeviceWrite(hUart, 0, strlen(string), (BT_u8 *)string);

	BT_CloseHandle(hUart);

	int retval = main(argc, argv);

	// Write a debug message to the debugger port,
	//	The main application has quit on us!

	return retval;
}
