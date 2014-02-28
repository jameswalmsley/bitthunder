#ifndef _PORTMACRO_H_
#define _PORTMACRO_H_

#include "bt_config.h"

#if (defined BT_CONFIG_ARCH_ARM_CORTEX_A9) && (defined BT_CONFIG_KERNEL_FREERTOS_CA9_MODERN_PORT)
#include <portmacro-ca9.h>
#else

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	unsigned portLONG
#define portBASE_TYPE	portLONG

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

typedef unsigned long UBaseType_t;
typedef unsigned long TickType_t;

//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS	portConfigureRuntimeTimer
#define portGET_RUN_TIME_COUNTER_VALUE			portGetRuntimeCounter

#define portMAX_DELAY ( TickType_t ) 0xffffffff

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH		( -1 )
#define portBYTE_ALIGNMENT		8
#define portNOP()				__asm volatile ( "NOP" );
/*-----------------------------------------------------------*/


/* Scheduler utilities. */

/*
 * portRESTORE_CONTEXT, portRESTORE_CONTEXT, portENTER_SWITCHING_ISR
 * and portEXIT_SWITCHING_ISR can only be called from ARM mode, but
 * are included here for efficiency.  An attempt to call one from
 * THUMB mode code will result in a compile time error.
 */
#if (defined BT_CONFIG_ARCH_ARM_CORTEX_A9) || defined (BT_CONFIG_ARCH_ARM_ARM11)
#ifdef BT_CONFIG_TOOLCHAIN_FLOAT_HARD
#define portRESTORE_FPU()												\
	/* Save the floating point state */									\
	"LDMFD		LR!, {R0}										\n\t"	\
	"VMSR		FPEXC, r0										\n\t"	\
	"LDMFD		LR!, {R0}										\n\t"	\
	"VMSR		FPSCR, r0										\n\t"	\
	"VLDMIA		LR!, {D16-D31}									\n\t"	\
	"VLDMIA		LR!, {D0-D15}									\n\t"

#define portSAVE_FPU()													\
	/* Save the FLOATING point state */									\
	"VSTMDB	LR!, {D0-D15}										\n\t"	\
	"VSTMDB	LR!, {D16-D31}										\n\t"	\
	"VMRS	R1, FPSCR											\n\t"	\
	"STMDB	LR!, {R1}											\n\t"	\
	"VMRS	R1, FPEXC											\n\t"	\
	"STMDB	LR!, {R1}											\n\t"
#else
#define portRESTORE_FPU()
#define portSAVE_FPU()
#endif

#define portRESTORE_CONTEXT()											\
{																		\
extern volatile void * volatile pxCurrentTCB;							\
extern volatile unsigned portLONG ulCriticalNesting;					\
																		\
	/* Set the LR to the task stack. */									\
	__asm volatile (													\
	"LDR		R0, =pxCurrentTCB								\n\t"	\
	"LDR		R0, [R0]										\n\t"	\
	"LDR		LR, [R0]										\n\t"	\
																		\
	                                                                    \
	/* The critical nesting depth is the first item on the stack. */	\
	/* Load it into the ulCriticalNesting variable. */					\
	"LDR		R0, =ulCriticalNesting							\n\t"	\
	"LDMFD		LR!, {R1}										\n\t"	\
	"STR		R1, [R0]										\n\t"	\
																		\
	/* Get the SPSR from the stack. */									\
	"LDMFD		LR!, {R0}										\n\t"	\
	"MSR		SPSR, R0										\n\t"	\
																		\
	portRESTORE_FPU()													\
	                                                                    \
	/* Restore all system mode registers for the task. */				\
	"LDMFD	LR, {R0-R14}^										\n\t"	\
	"NOP														\n\t"	\
																		\
	/* Restore the return address. */									\
	"LDR		LR, [LR, #+60]									\n\t"	\
																		\
	/* And return - correcting the offset in the LR to obtain the */	\
	/* correct address. */												\
	"SUBS	PC, LR, #4											\n\t"	\
	"NOP														\n\t"	\
	"NOP														\n\t"	\
	);																	\
	( void ) ulCriticalNesting;											\
	( void ) pxCurrentTCB;												\
}
/*-----------------------------------------------------------*/

#define portSAVE_CONTEXT()												\
{																		\
extern volatile void * volatile pxCurrentTCB;							\
extern volatile unsigned portLONG ulCriticalNesting;					\
																		\
	/* Push R0 as we are going to use the register. */					\
	__asm volatile (													\
	"STMDB	SP!, {R0}											\n\t"	\
	/* Set R0 to point to the task stack pointer. */					\
	"STMDB	SP,{SP}^											\n\t"	\
	"NOP														\n\t"	\
	"SUB	SP, SP, #4											\n\t"	\
	"LDMIA	SP!,{R0}											\n\t"	\
																		\
	/* Push the return address onto the stack. */						\
	"STMDB	R0!, {LR}											\n\t"	\
																		\
	/* Now we have saved LR we can use it instead of R0. */				\
	"MOV	LR, R0												\n\t"	\
																		\
	/* Pop R0 so we can save it onto the system mode stack. */			\
	"LDMIA	SP!, {R0}											\n\t"	\
																		\
	/* Push all the system mode registers onto the task stack. */		\
	"STMDB	LR,{R0-LR}^											\n\t"	\
	"NOP														\n\t"	\
	"SUB	LR, LR, #60											\n\t"	\
	                                                                    \
	portSAVE_FPU()														\
																		\
	/* Push the SPSR onto the task stack. */							\
	"MRS	R0, SPSR											\n\t"	\
	"STMDB	LR!, {R0}											\n\t"	\
																		\
	"LDR	R0, =ulCriticalNesting								\n\t"	\
	"LDR	R0, [R0]											\n\t"	\
	"STMDB	LR!, {R0}											\n\t"	\
																		\
	/* Store the new top of stack for the task. */						\
	"LDR	R0, =pxCurrentTCB									\n\t"	\
	"LDR	R0, [R0]											\n\t"	\
	"STR	LR, [R0]											\n\t"	\
	);																	\
	( void ) ulCriticalNesting;											\
	( void ) pxCurrentTCB;												\
}
#endif

extern void vTaskSwitchContext( void );

#define portYIELD_FROM_ISR()		vTaskSwitchContext()

/*-----------------------------------------------------------*/

#if (defined BT_CONFIG_ARCH_ARM_CORTEX_A9) || (defined BT_CONFIG_ARCH_ARM_ARM11)
/* Critical section management. */

	#define portDISABLE_INTERRUPTS()											\
		__asm volatile (														\
			"STMDB	SP!, {R0}		\n\t"	/* Push R0.						*/	\
			"MRS	R0, CPSR		\n\t"	/* Get CPSR.					*/	\
			"ORR	R0, R0, #0xC0	\n\t"	/* Disable IRQ, FIQ.			*/	\
			"MSR	CPSR, R0		\n\t"	/* Write back modified value.	*/	\
			"LDMIA	SP!, {R0}			" )	/* Pop R0.						*/

	#define portENABLE_INTERRUPTS()												\
		__asm volatile (														\
			"STMDB	SP!, {R0}		\n\t"	/* Push R0.						*/	\
			"MRS	R0, CPSR		\n\t"	/* Get CPSR.					*/	\
			"BIC	R0, R0, #0xC0	\n\t"	/* Enable IRQ, FIQ.				*/	\
			"MSR	CPSR, R0		\n\t"	/* Write back modified value.	*/	\
			"LDMIA	SP!, {R0}			" )	/* Pop R0.						*/

#endif

#if (defined BT_CONFIG_ARCH_ARM_CORTEX_M0) || (defined BT_CONFIG_ARCH_ARM_CORTEX_M3)

#ifdef BT_CONFIG_ARCH_ARM_CORTEX_M0
	/*
	 * Set basepri to portMAX_SYSCALL_INTERRUPT_PRIORITY without effecting other
	 * registers.  r0 is clobbered.
	 */
	#define portSET_INTERRUPT_MASK()						__asm volatile  (  " cpsid i " )
	#define portCLEAR_INTERRUPT_MASK()						__asm volatile	(  " cpsie i " )
#endif

#ifdef BT_CONFIG_ARCH_ARM_CORTEX_M3
	/*
	 * Set basepri to portMAX_SYSCALL_INTERRUPT_PRIORITY without effecting other
	 * registers.  r0 is clobbered.
	 */
	#define portSET_INTERRUPT_MASK()						\
		__asm volatile										\
		(													\
			"	mov r0, %0								\n"	\
			"	msr basepri, r0							\n" \
			::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY):"r0"	\
		)

	/*
	 * Set basepri back to 0 without effective other registers.
	 * r0 is clobbered.  FAQ:  Setting BASEPRI to 0 is not a bug.  Please see
	 * http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html before disagreeing.
	 */
	#define portCLEAR_INTERRUPT_MASK()			\
		__asm volatile							\
		(										\
			"	mov r0, #0					\n"	\
			"	msr basepri, r0				\n"	\
			:::"r0"								\
		)

#endif



/* FAQ:  Setting BASEPRI to 0 in portCLEAR_INTERRUPT_MASK_FROM_ISR() is not a
bug.  Please see http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html before
disagreeing. */
#define portSET_INTERRUPT_MASK_FROM_ISR()		0;portSET_INTERRUPT_MASK()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	portCLEAR_INTERRUPT_MASK();(void)x


#define portDISABLE_INTERRUPTS()	portSET_INTERRUPT_MASK()
#define portENABLE_INTERRUPTS()		portCLEAR_INTERRUPT_MASK()

	extern void vPortYieldFromISR( void );
	#define portYIELD()					vPortYieldFromISR()
#else
	#define portYIELD()					__asm volatile ( "SWI 0x00FF0000" )
#endif

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
extern void vPortReset( void );

#define portENTER_CRITICAL()		vPortEnterCritical();
#define portEXIT_CRITICAL()			vPortExitCritical();
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

void * prvGetInterruptControllerInstance( void );

void prvInitializeExceptions( void );

void vApplicationSetupHardware( void );



#ifdef __cplusplus
}
#endif

#endif
#endif
