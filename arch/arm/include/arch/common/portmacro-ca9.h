/**
 *	Port Macros for Cortex-A9 processor.
 *
 **/

#ifndef _PORTMACRO_CA9_H_
#define _PORTMACRO_CA9_H_


/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the given hardware
 * and compiler.
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
#define portSTACK_TYPE	unsigned long
#define portBASE_TYPE	portLONG

#if( configUSE_16_BIT_TICKS == 1 )
	typedef unsigned short portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffff
#else
	typedef unsigned long portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffffffffUL
#endif
/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portSTACK_GROWTH			( -1 )
#define portTICK_RATE_MS			( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT			8

/*-----------------------------------------------------------*/

/* Task utilities. */

/* Called at the end of an ISR that can cause a context switch. */
#define portEND_SWITCHING_ISR( xSwitchRequired )\
{												\
extern unsigned long ulPortYieldRequired;		\
												\
	if( xSwitchRequired != pdFALSE )			\
	{											\
		ulPortYieldRequired = pdTRUE;			\
	}											\
}

#define portYIELD_FROM_ISR( x ) portEND_SWITCHING_ISR( x )
#define portYIELD() __asm volatile ( "SVC 0xff" );


/*-----------------------------------------------------------
 * Critical section control
 *----------------------------------------------------------*/

extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
extern unsigned long ulPortSetInterruptMask( void );
extern void vPortClearInterruptMask( unsigned long ulNewMaskValue );

/* These macros do not globally disable/enable interrupts.  They do mask off
interrupts that have a priority below configMAX_API_CALL_INTERRUPT_PRIORITY. */
#define portENTER_CRITICAL()		vPortEnterCritical();
#define portEXIT_CRITICAL()			vPortExitCritical();
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



/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not required for this port but included in case common demo code that uses these
macros is used. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )	void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )	void vFunction( void *pvParameters )

/* Prototype of the FreeRTOS tick handler.  This must be installed as the
handler for whichever peripheral is used to generate the RTOS tick. */
void FreeRTOS_Tick_Handler( void );

/* Any task that uses the floating point unit MUST call vPortTaskUsesFPU()
before any floating point instructions are executed. */
void vPortTaskUsesFPU( void );
#define portTASK_USES_FLOATING_POINT() vPortTaskUsesFPU()

#define portLOWEST_INTERRUPT_PRIORITY ( ( ( unsigned long ) configUNIQUE_INTERRUPT_PRIORITIES ) - 1UL )
#define portLOWEST_USABLE_INTERRUPT_PRIORITY ( portLOWEST_INTERRUPT_PRIORITY - 1UL )

/* Architecture specific optimisations. */
#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* Store/clear the ready priorities in a bit map. */
	#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
	#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

	/*-----------------------------------------------------------*/

	#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31 - __clz( uxReadyPriorities ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

#ifdef configASSERT
	void vPortValidateInterruptPriority( void );
	#define portASSERT_IF_INTERRUPT_PRIORITY_INVALID() 	vPortValidateInterruptPriority()
#endif

#define portNOP() __nop()

#endif
