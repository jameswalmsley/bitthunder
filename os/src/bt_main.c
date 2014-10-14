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
#include <of/bt_of.h>

static bt_kernel_params g_kernel_params = { NULL };

int main(int argc, char **argv);

void bt_do_bug(void *pc) {
#ifndef BT_CONFIG_KERNEL_NONE
	BT_StopSystemTimer();
#endif
	BT_DisableInterrupts();

	BT_kPrint("%p, 0x%08x",pc,*((BT_u32 *)pc));
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

int bt_main(BT_u32 machid, const void *fdt) {

	BT_ERROR 	Error;
	BT_HANDLE 	hUart = NULL;

	g_kernel_params.cmdline = "";

#ifdef BT_CONFIG_OF
	g_kernel_params.fdt = fdt;
	BT_LIST_INIT_HEAD(&g_kernel_params.devices);
#endif

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
#ifdef BT_CONFIG_MEM_PAGE_ALLOCATOR
	g_kernel_params.coherent = bt_initialise_coherent_pages();
#endif
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	bt_vm_init();
#endif

#ifdef BT_CONFIG_OF
	bt_of_init();
#endif

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(pMachine->szpName) {
		Error = 0;
	}

	if (pMachine->pfnMachineInit) {
		pMachine->pfnMachineInit(pMachine);
	}

	bt_process_init();

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

#ifdef BT_CONFIG_OF
	if(!hUart) {
		struct bt_device_node *logger = bt_of_get_bootlogger();
		pDriver = BT_GetIntegratedDriverByName(logger->dev.name);
		if(pDriver) {
			hUart = pDriver->pfnProbe(&logger->dev, &Error);
		}
	}
#endif

	BT_UART_CONFIG oConfig;
	BT_SetPowerState(hUart, BT_POWER_STATE_AWAKE);

	oConfig.eMode			= BT_UART_MODE_POLLED;
	oConfig.ucDataBits		= BT_UART_8_DATABITS;
	oConfig.ucStopBits		= BT_UART_ONE_STOP_BIT;
	oConfig.ucParity		= BT_UART_PARITY_NONE;
	oConfig.ulBaudrate		= 115200;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_UartEnable(hUart);

	BT_SetStdin(hUart);
	BT_SetStdout(hUart);
	BT_SetStderr(hUart);

	BT_kPrint("%s (%s)", BT_VERSION_STRING, BT_VERSION_NAME);

	BT_kPrint("Command line: %s", g_kernel_params.cmdline);

	BT_kPrint("Start Loading kernel modules...");
	Error = BT_InitialiseKernelModules(hUart);

#ifdef BT_CONFIG_CACHE_MAINTENANCE
	BT_DCacheFlush();	// Flush the cache now, to ensure all memory pools are coherent.
#endif

	BT_kPrint("Enumerate integrated devices");
	Error = BT_ProbeIntegratedDevices(hUart);

	BT_kPrint("Enter user-mode, and start user-space application...");

	BT_kPrint("Relinquish control of the boot UART device...(Goodbye)");

	BT_Flush(hUart);

#ifndef BT_CONFIG_INHERIT_STDIO_FROM_KERNEL
	BT_SetStdin(NULL);
	BT_SetStdout(NULL);
	BT_SetStderr(NULL);

	if (hUart) {
		BT_CloseHandle(hUart);
	}
#endif

#ifndef BT_CONFIG_KERNEL_NONE
	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth 	= BT_CONFIG_MAIN_TASK_STACK_DEPTH,
		.ulPriority		= BT_CONFIG_MAIN_TASK_PRIORITY,
	};

	BT_CreateProcess((BT_FN_THREAD_ENTRY) main, "MAIN", &oThreadConfig, &Error);

	BT_StartScheduler();
#else
	main(0, NULL);
#endif

	// Write a debug message to the debugger port,
	// It was not possible to start the scheduler.

	return BT_ERR_NONE;
}

bt_kernel_params *bt_get_kernel_params() {
	return &g_kernel_params;
}
BT_EXPORT_SYMBOL(bt_get_kernel_params);


__BT_WEAK int main(int argc, char **argv) {

#ifndef BT_CONFIG_INHERIT_STDIO_FROM_KERNEL
	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hUart = NULL;
	const BT_INTEGRATED_DRIVER *pDriver = NULL;

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(pMachine->szpName) {
		Error = 0;
	}

	if (pMachine->pBootLogger) {
		pDriver = BT_GetIntegratedDriverByName(pMachine->pBootLogger->name);
		if(pDriver) {
			hUart = pDriver->pfnProbe(pMachine->pBootLogger, &Error);
		}
	}

#ifdef BT_CONFIG_OF
	if(!hUart) {
		struct bt_device_node *logger = bt_of_get_bootlogger();
		pDriver = BT_GetIntegratedDriverByName(logger->dev.name);
		if(pDriver) {
			hUart = pDriver->pfnProbe(&logger->dev, &Error);
		}
	}
#endif

	BT_UART_CONFIG oConfig;
	BT_SetPowerState(hUart, BT_POWER_STATE_AWAKE);

	oConfig.eMode			= BT_UART_MODE_POLLED;
	oConfig.ucDataBits		= BT_UART_8_DATABITS;
	oConfig.ucStopBits		= BT_UART_ONE_STOP_BIT;
	oConfig.ucParity		= BT_UART_PARITY_NONE;
	oConfig.ulBaudrate		= 115200;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_UartEnable(hUart);
	BT_SetStdout(hUart);
#endif

	while(1) {
		BT_kPrint("Welcome to BitThunder");
		BT_ThreadSleep(1000);
	}
}
