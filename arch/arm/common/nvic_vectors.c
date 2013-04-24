/**
 *	Provides a generic NVIC vector table with weak symbols in C.
 *
 *	Any driver can override these weak symbols, which means that the table
 *	does not need to be re-located, unless the table changes during runtime.
 *
 *	@author James Walmsley <james@fullfat-fs.co.uk>
 **/

#include <bitthunder.h>
#include "arch/common/scb.h"

extern void _stack(void);

/**
 *	Give all of the symbols weak attributes.
 **/
void BT_NVIC_Reset_Handler(void);// The reset handler should not be patchable at runtime!
void __attribute__((weak)) BT_NVIC_NMI_Handler(void);
void __attribute__((weak)) BT_NVIC_HardFault_Handler(void);
void __attribute__((weak)) BT_NVIC_MemManage_Handler(void);
void __attribute__((weak)) BT_NVIC_BusFault_Handler(void);
void __attribute__((weak)) BT_NVIC_UsageFault_Handler(void);
void __attribute__((weak)) BT_NVIC_SVC_Handler(void);
void __attribute__((weak)) BT_NVIC_DebugMon_Handler(void);
void __attribute__((weak)) BT_NVIC_PendSV_Handler(void);
void __attribute__((weak)) BT_NVIC_SysTick_Handler(void);

#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 16
	void __attribute__((weak)) BT_NVIC_IRQ_16(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 17
	void __attribute__((weak)) BT_NVIC_IRQ_17(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 18
	void __attribute__((weak)) BT_NVIC_IRQ_18(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 19
	void __attribute__((weak)) BT_NVIC_IRQ_19(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 20
	void __attribute__((weak)) BT_NVIC_IRQ_20(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 21
	void __attribute__((weak)) BT_NVIC_IRQ_21(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 22
	void __attribute__((weak)) BT_NVIC_IRQ_22(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 23
	void __attribute__((weak)) BT_NVIC_IRQ_23(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 24
	void __attribute__((weak)) BT_NVIC_IRQ_24(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 25
	void __attribute__((weak)) BT_NVIC_IRQ_25(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 26
	void __attribute__((weak)) BT_NVIC_IRQ_26(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 27
	void __attribute__((weak)) BT_NVIC_IRQ_27(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 28
	void __attribute__((weak)) BT_NVIC_IRQ_28(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 29
	void __attribute__((weak)) BT_NVIC_IRQ_29(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 30
	void __attribute__((weak)) BT_NVIC_IRQ_30(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 31
	void __attribute__((weak)) BT_NVIC_IRQ_31(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 32
	void __attribute__((weak)) BT_NVIC_IRQ_32(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 33
	void __attribute__((weak)) BT_NVIC_IRQ_33(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 34
	void __attribute__((weak)) BT_NVIC_IRQ_34(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 35
	void __attribute__((weak)) BT_NVIC_IRQ_35(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 36
	void __attribute__((weak)) BT_NVIC_IRQ_36(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 37
	void __attribute__((weak)) BT_NVIC_IRQ_37(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 38
	void __attribute__((weak)) BT_NVIC_IRQ_38(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 39
	void __attribute__((weak)) BT_NVIC_IRQ_39(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 40
	void __attribute__((weak)) BT_NVIC_IRQ_40(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 41
	void __attribute__((weak)) BT_NVIC_IRQ_41(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 42
	void __attribute__((weak)) BT_NVIC_IRQ_42(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 43
	void __attribute__((weak)) BT_NVIC_IRQ_43(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 44
	void __attribute__((weak)) BT_NVIC_IRQ_44(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 45
	void __attribute__((weak)) BT_NVIC_IRQ_45(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 46
	void __attribute__((weak)) BT_NVIC_IRQ_46(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 47
	void __attribute__((weak)) BT_NVIC_IRQ_47(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 48
	void __attribute__((weak)) BT_NVIC_IRQ_48(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 49
	void __attribute__((weak)) BT_NVIC_IRQ_49(void);
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 50
	void __attribute__((weak)) BT_NVIC_IRQ_50(void);
#endif
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
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 16
	BT_NVIC_IRQ_16,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 17
	BT_NVIC_IRQ_17,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 18
	BT_NVIC_IRQ_18,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 19
	BT_NVIC_IRQ_19,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 20
	BT_NVIC_IRQ_20,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 21
	BT_NVIC_IRQ_21,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 22
	BT_NVIC_IRQ_22,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 23
	BT_NVIC_IRQ_23,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 24
	BT_NVIC_IRQ_24,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 25
	BT_NVIC_IRQ_25,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 26
	BT_NVIC_IRQ_26,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 27
	BT_NVIC_IRQ_27,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 28
	BT_NVIC_IRQ_28,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 29
	BT_NVIC_IRQ_29,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 30
	BT_NVIC_IRQ_30,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 31
	BT_NVIC_IRQ_31,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 32
	BT_NVIC_IRQ_32,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 33
	BT_NVIC_IRQ_33,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 34
	BT_NVIC_IRQ_34,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 35
	BT_NVIC_IRQ_35,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 36
	BT_NVIC_IRQ_36,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 37
	BT_NVIC_IRQ_37,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 38
	BT_NVIC_IRQ_38,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 39
	BT_NVIC_IRQ_39,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 40
	BT_NVIC_IRQ_40,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 41
	BT_NVIC_IRQ_41,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 42
	BT_NVIC_IRQ_42,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 43
	BT_NVIC_IRQ_43,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 44
	BT_NVIC_IRQ_44,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 45
	BT_NVIC_IRQ_45,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 46
	BT_NVIC_IRQ_46,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 47
	BT_NVIC_IRQ_47,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 48
	BT_NVIC_IRQ_48,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 49
	BT_NVIC_IRQ_49,
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 50
	BT_NVIC_IRQ_50,
#endif
};

void BT_NVIC_Default_Handlr(void);
void BT_NVIC_Test_Handler(void);
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
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 16
	#pragma weak BT_NVIC_IRQ_16			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 17
	#pragma weak BT_NVIC_IRQ_17			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 18
	#pragma weak BT_NVIC_IRQ_18			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 19
	#pragma weak BT_NVIC_IRQ_19			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 20
	#pragma weak BT_NVIC_IRQ_20			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 21
	#pragma weak BT_NVIC_IRQ_21			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 22
	#pragma weak BT_NVIC_IRQ_22			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 23
	#pragma weak BT_NVIC_IRQ_23			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 24
	#pragma weak BT_NVIC_IRQ_24			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 25
	#pragma weak BT_NVIC_IRQ_25			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 26
	#pragma weak BT_NVIC_IRQ_26			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 27
	#pragma weak BT_NVIC_IRQ_27			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 28
	#pragma weak BT_NVIC_IRQ_28			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 29
	#pragma weak BT_NVIC_IRQ_29			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 30
	#pragma weak BT_NVIC_IRQ_30			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 31
	#pragma weak BT_NVIC_IRQ_31			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 32
	#pragma weak BT_NVIC_IRQ_32			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 33
	#pragma weak BT_NVIC_IRQ_33			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 34
	#pragma weak BT_NVIC_IRQ_34			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 35
	#pragma weak BT_NVIC_IRQ_35			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 36
	#pragma weak BT_NVIC_IRQ_36			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 37
	#pragma weak BT_NVIC_IRQ_37			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 38
	#pragma weak BT_NVIC_IRQ_38			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 39
	#pragma weak BT_NVIC_IRQ_39			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 40
	#pragma weak BT_NVIC_IRQ_40			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 41
	#pragma weak BT_NVIC_IRQ_41			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 42
	#pragma weak BT_NVIC_IRQ_42			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 43
	#pragma weak BT_NVIC_IRQ_43			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 44
	#pragma weak BT_NVIC_IRQ_44			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 45
	#pragma weak BT_NVIC_IRQ_45			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 46
	#pragma weak BT_NVIC_IRQ_46			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 47
	#pragma weak BT_NVIC_IRQ_47			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 48
	#pragma weak BT_NVIC_IRQ_48			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 49
	#pragma weak BT_NVIC_IRQ_49			= BT_NVIC_Default_Handler
#endif
#if BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS > 50
	#pragma weak BT_NVIC_IRQ_50			= BT_NVIC_Default_Handler
#endif

void BT_NVIC_Default_Handler(void) {
	return;
	while (1)
		;
}

void BT_NVIC_Test_Handler(void) {
	while (1);
}

void bt_startup_default_hook(void) {
	return;
}

extern int bt_main(int argc, char **argv);

void bt_startup_boot(void) {
	bt_main(0, NULL );
	while (1)
		;
}

void __attribute__((weak)) bt_startup_init_hook(void);
void __attribute__((weak)) bt_startup_boot(void);

#pragma weak bt_startup_init_hook = bt_startup_default_hook

extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long __data_start;
extern BT_u32 __bt_init_start;
extern unsigned long __data_end;
extern unsigned long _bss_begin;
extern unsigned long _bss_end;
extern unsigned long _estack;

void BT_NVIC_Reset_Handler(void) {
	SCB_REGS * pSCB = SCB;

	bt_startup_init_hook();

	pSCB->VTOR = (BT_u32) &__bt_init_start;		// point to flash position 0

	BT_u32 *pSrc;
	BT_u32 *pDest;

	pSrc = &_etext;
	pDest = &__data_start;

	while (pDest < &__data_end) {
		*pDest++ = *pSrc++;
	}

	pDest = &_bss_begin;
	while (pDest < &_bss_end) {
		*pDest++ = 0;
	}

	bt_startup_boot();

	while (1)
		;
}

