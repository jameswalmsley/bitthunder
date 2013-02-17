/**
 *	Provides a generic NVIC vector table with weak symbols in C.
 *
 *	Any driver can override these weak symbols, which means that the table
 *	does not need to be re-located, unless the table changes during runtime.
 *
 *	@author James Walmsley <james@fullfat-fs.co.uk>
 **/

#include <bitthunder.h>

extern void _stack(void);

/**
 *	Give all of the symbols weak attributes.
 **/
void		  	  	   BT_NVIC_Reset_Handler		(void);	// The reset handler should not be patchable at runtime!
void __attribute__((weak)) BT_NVIC_NMI_Handler			(void);
void __attribute__((weak)) BT_NVIC_HardFault_Handler	(void);
void __attribute__((weak)) BT_NVIC_MemManage_Handler	(void);
void __attribute__((weak)) BT_NVIC_BusFault_Handler		(void);
void __attribute__((weak)) BT_NVIC_UsageFault_Handler	(void);
void __attribute__((weak)) BT_NVIC_SVC_Handler			(void);
void __attribute__((weak)) BT_NVIC_DebugMon_Handler		(void);
void __attribute__((weak)) BT_NVIC_PendSV_Handler		(void);
void __attribute__((weak)) BT_NVIC_SysTick_Handler		(void);


/**
 *	Here we define the cortex-m3 vector table.
 *	We can use the pre-processor to extend the optional entries.
 **/
__attribute__((section(".bt.init.vectors")))
void (* const g_pfnVectors[])(void) = {
	&_stack,
	BT_NVIC_Reset_Handler,
	BT_NVIC_NMI_Handler,
	BT_NVIC_HardFault_Handler,
	BT_NVIC_MemManage_Handler,
	BT_NVIC_BusFault_Handler,
	BT_NVIC_UsageFault_Handler,
	0,
	0,
	0,
	0,
	BT_NVIC_SVC_Handler,
	BT_NVIC_DebugMon_Handler,
	0,
	BT_NVIC_PendSV_Handler,
	BT_NVIC_SysTick_Handler,
};


void BT_NVIC_Default_Handler(void);
//#pragma weak BT_NVIC_Reset_Handler		= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_NMI_Handler		= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_HardFault_Handler	= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_MemManage_Handler	= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_BusFault_Handler	= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_UsageFault_Handler	= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_SVC_Handler		= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_DebugMon_Handler	= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_PendSV_Handler		= BT_NVIC_Default_Handler 
#pragma weak BT_NVIC_SysTick_Handler	= BT_NVIC_Default_Handler 

void BT_NVIC_Default_Handler(void) {
	return;
	while(1);
}


void bt_startup_default_hook(void) {
	return;
}


extern int bt_main(int argc, char **argv);

void bt_startup_boot(void) {
	bt_main(0, NULL);
	while(1);
}

void __attribute__((weak)) bt_startup_init_hook(void);
void __attribute__((weak)) bt_startup_boot(void);

#pragma weak bt_startup_init_hook = bt_startup_default_hook

extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long __data_start;
extern unsigned long __data_end;
extern unsigned long _bss_begin;
extern unsigned long _bss_end;
extern unsigned long _estack;

void BT_NVIC_Reset_Handler(void) {
	bt_startup_init_hook();

	*((BT_u32*)0xE000ED08) = (BT_u32) 0x08000000;
	
	BT_u32 *pSrc;
	BT_u32 *pDest;

	pSrc = &_etext;
	pDest = &__data_start;

	while(pDest < &__data_end) {
		*pDest++ = *pSrc++;
	}

	pDest = &_bss_begin;
	while(pDest < &_bss_end) {
		*pDest++ = 0;
	}

	bt_startup_boot();

	while(1);
}

