/**
 *	This is the operating system main entry point, from where it will boot and load
 *	kernel modules.
 *
 *	Once all kernel modules are loaded and initialised, we shall look for user application
 *	processes to be loaded.
 *
 **/

#include <bitthunder.h>
#include <mm/bt_heap.h>
#include <mm/bt_page.h>
#include <string.h>
#include <bt_kernel.h>
#include <lib/putc.h>

extern int main(int argc, char **argv);

void bt_do_bug(void *pc) {
#ifndef BT_CONFIG_KERNEL_NONE
	BT_StopSystemTimer();
#endif
	BT_DisableInterrupts();

	BT_kPrint("Step 1. DO NOT PANIC!");
	BT_kPrint("Step 2. Take a deep breath");
	BT_kPrint("Step 3. Read the bad news below:");
	BT_kPrint("=============================================================");
	BT_kPrint("Kernel Panic: Segmentation fault! at %p", pc);
	BT_kPrint("Lookup the address in your kernel.list file.........");
	BT_kPrint("=============================================================");
	BT_kPrint("Step 4. Grab a coffee and know that all will be well!");

	while(1);
}

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

/**
 *	@ Note: argc and argv may be used for kernel booting parameters at a later date.
 **/
int bt_main(int argc, char **argv) {

	BT_ERROR 	Error;
	BT_HANDLE 	hUart = NULL;

#ifdef BT_CONFIG_MEM_PAGE_ALLOCATOR
	bt_initialise_pages();
#endif
#ifdef BT_CONFIG_MEM_SLAB_ALLOCATOR
	bt_initialise_slab();
	bt_initialise_slab_second_stage();
#endif
#ifdef BT_CONFIG_MEM_PAGE_ALLOCATOR
	bt_initialise_pages_second_stage();
#endif
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING	
	bt_vm_init();
#endif

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(pMachine->szpName) {
		Error = 0;
	}

	if (pMachine->pfnMachineInit) {
		pMachine->pfnMachineInit(pMachine);
	}

	const BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pMachine->pInterruptController->name);
	if(pDriver) {
		pDriver->pfnProbe(pMachine->pInterruptController, &Error);
	}

	if (pMachine->pBootLogger)
	{
		pDriver = BT_GetIntegratedDriverByName(pMachine->pBootLogger->name);
		if(pDriver) {
			hUart = pDriver->pfnProbe(pMachine->pBootLogger, &Error);
		}
	}

	BT_UART_CONFIG oConfig;
	BT_SetPowerState(hUart, BT_POWER_STATE_AWAKE);

	oConfig.eMode			= BT_UART_MODE_POLLED;
	oConfig.ucDataBits		= BT_UART_8_DATABITS;
	oConfig.ucStopBits		= BT_UART_ONE_STOP_BIT;
	oConfig.ucParity		= BT_UART_PARITY_NONE;
	oConfig.ulBaudrate		= 115200;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_UartEnable(hUart);

	BT_SetStandardHandle(hUart);

	BT_kPrint("%s (%s)", BT_VERSION_STRING, BT_VERSION_NAME);

	BT_kPrint("Start Loading kernel modules...");
	Error = BT_InitialiseKernelModules(hUart);

	BT_kPrint("Enumerate integrated devices");
	Error = BT_ProbeIntegratedDevices(hUart);

	BT_kPrint("Enter user-mode, and start user-space application...");

	BT_kPrint("Relinquish control of the boot UART device...(Goodbye)");

#ifndef BT_CONFIG_KERNEL_NONE
	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth 	= BT_CONFIG_MAIN_TASK_STACK_DEPTH,
		.ulPriority		= BT_CONFIG_MAIN_TASK_PRIORITY,
	};

	BT_CreateProcess((BT_FN_THREAD_ENTRY) main, "MAIN", &oThreadConfig, &Error);

	BT_Flush(hUart);

	BT_SetStandardHandle(NULL);
	if (hUart) {
		BT_CloseHandle(hUart);
	}

	BT_kStartScheduler();
#endif

	main(0, NULL);

	// Write a debug message to the debugger port,
	// It was not possible to start the scheduler.

	return BT_ERR_NONE;
}
