/**
 *	Copyright (c) 2013 James Walmsley
 *
 *
 **/

#include <bitthunder.h>
#include "bt_types.h"

#include <FreeRTOS.h>
#include <task.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING		( ( unsigned long ) 0 )
volatile unsigned long ulCriticalNesting = 9999UL;

static BT_ERROR tick_isr_handler(BT_u32 ulIRQ, void *pParam) {
	vTaskIncrementTick();

#if configUSE_PREEMPTION == 1
	vTaskSwitchContext();
#endif

	// Timer interrupt status is handled automatically by BitThunder driver.
	return BT_ERR_NONE;
}

/* Constants required to setup the task context. */
/* System mode, ARM mode, interrupts enabled. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x1f )
#define portTHUMB_MODE_BIT				( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )
#define portNO_CRITICAL_SECTION_NESTING	( ( portSTACK_TYPE ) 0 )

/**
 *	Initialise the stack of a task to look as if a call to
 *	portSAVE_CONTEXT has been called.
 *
 **/

portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack,
									   pdTASK_CODE pxCode, void *pvParameters )
{
	portSTACK_TYPE *pxOriginalTOS;
	pxOriginalTOS = pxTopOfStack;

	/* Setup the initial stack of the task.  The stack is set exactly as
	expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	start of the task.  The offset is added to make the return address appear
	as it would within an IRQ ISR. */
	*pxTopOfStack = ( portSTACK_TYPE ) pxCode + portINSTRUCTION_SIZE;
	pxTopOfStack--;

	*pxTopOfStack = ( portSTACK_TYPE ) xPortStartScheduler;	/* R14 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) pxOriginalTOS;
								/* Stack used when task starts goes in R13. */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x12121212;	/* R12 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x11111111;	/* R11 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x10101010;	/* R10 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x09090909;	/* R9 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x08080808;	/* R8 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x07070707;	/* R7 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x06060606;	/* R6 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x05050505;	/* R5 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x04040404;	/* R4 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x03030303;	/* R3 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x02020202;	/* R2 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x01010101;	/* R1 */
	pxTopOfStack--;

	/* When the task starts is will expect to find the function parameter in
	R0. */
	*pxTopOfStack = ( portSTACK_TYPE ) pvParameters; /* R0 */
	pxTopOfStack--;

	/* The last thing onto the stack is the status register, which is set for
	system mode, with interrupts enabled. */
	*pxTopOfStack = ( portSTACK_TYPE ) portINITIAL_SPSR;

	if( ( ( unsigned long ) pxCode & 0x01UL ) != 0x00 )
	{
		/* We want the task to start in thumb mode. */
		*pxTopOfStack |= portTHUMB_MODE_BIT;
	}

	pxTopOfStack--;

	/* Some optimisation levels use the stack differently to others.  This
	means the interrupt flags cannot always be stored on the stack and will
	instead be stored in a variable, which is then saved as part of the
	tasks context. */
	*pxTopOfStack = portNO_CRITICAL_SECTION_NESTING;

	return pxTopOfStack;
}

void vPortExitTask( void ) {
	vTaskDelete(NULL);
}


static void prvSetupTimerInterrupt(void) {

	BT_ERROR Error;
	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(!pMachine) {
		return;
	}

	const BT_INTEGRATED_DRIVER *pDriver;
	BT_HANDLE hTimer = NULL;

	pDriver = BT_GetIntegratedDriverByName(pMachine->pSystemTimer->name);
	if(!pDriver) {
		return;
	}

	hTimer = pDriver->pfnProbe(pMachine->pSystemTimer, &Error);
	if(!hTimer) {
		return;
	}

	BT_SetSystemTimerHandle(hTimer);

	const BT_DEV_IF_SYSTIMER *pTimerOps = hTimer->h.pIf->oIfs.pDevIF->unConfigIfs.pSysTimerIF;
	pTimerOps->pfnSetFrequency(hTimer, configTICK_RATE_HZ);
	pTimerOps->pfnRegisterInterrupt(hTimer, tick_isr_handler, NULL);
	pTimerOps->pfnEnableInterrupt(hTimer);
	pTimerOps->pfnStart(hTimer);
}


extern void enable_irq(void);

void vPortISRStartFirstTask( void )
{

	/* Change from System to IRQ mode.
	 * portRESTORE_CONTEXT sets the desired CPSR by modifying SPSR, which
	 * requires that the processor be in an Exception mode to actually
	 * do anything by leaving that mode.
	 */
	__asm volatile (
		"LDR    R0, =0x1D2       \n\t"
		"MSR    CPSR, R0         \n\t"
		:::"r0" \
	);

	enable_irq();

	/* Simply start the scheduler.  This is included here as it can only be
	called from ARM mode. */
	portRESTORE_CONTEXT();
}

portBASE_TYPE xPortStartScheduler(void) {
	// Setup Hardware Timer!
	prvSetupTimerInterrupt();
	// Start first task

	vPortISRStartFirstTask();

	// Should not get here!
	return 0;
}

/*
 *	Critical Sections.
 *
 **/

/* The code generated by the GCC compiler uses the stack in different ways at
different optimisation levels.  The interrupt flags can therefore not always
be saved to the stack.  Instead the critical section nesting level is stored
in a variable, which is then saved as part of the stack context. */
void vPortEnterCritical( void )
{
	/* Disable interrupts as per portDISABLE_INTERRUPTS(); 					*/
	__asm volatile (
		"STMDB	SP!, {R0}			\n\t"	/* Push R0.						*/
		"MRS	R0, CPSR			\n\t"	/* Get CPSR.					*/
		"ORR	R0, R0, #0xC0		\n\t"	/* Disable IRQ, FIQ.			*/
		"MSR	CPSR, R0			\n\t"	/* Write back modified value.	*/
		"LDMIA	SP!, {R0}" );				/* Pop R0.						*/

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	directly.  Increment ulCriticalNesting to keep a count of how many times
	portENTER_CRITICAL() has been called. */
	ulCriticalNesting++;
}

void vPortExitCritical( void )
{
	if( ulCriticalNesting > portNO_CRITICAL_NESTING )
	{
		/* Decrement the nesting count as we are leaving a critical section. */
		ulCriticalNesting--;

		/* If the nesting level has reached zero then interrupts should be
		re-enabled. */
		if( ulCriticalNesting == portNO_CRITICAL_NESTING )
		{
			/* Enable interrupts as per portEXIT_CRITICAL().				*/
			__asm volatile (
				"STMDB	SP!, {R0}		\n\t"	/* Push R0.					*/
				"MRS	R0, CPSR		\n\t"	/* Get CPSR.				*/
				"BIC	R0, R0, #0xC0	\n\t"	/* Enable IRQ, FIQ.			*/
				"MSR	CPSR, R0		\n\t"	/* Write back modified value.*/
				"LDMIA	SP!, {R0}" );			/* Pop R0.					*/
		}
	}
}


void vPortEndScheduler( void )
{
	/* It is unlikely that the ARM port will require this function as there
	is nothing to return to.  */
}


void vPortYieldProcessor( void ) __attribute__((interrupt("SWI"), naked));

void vPortYieldProcessor( void )
{

	/* Within an IRQ ISR the link register has an offset from the true return
	address, but an SWI ISR does not.  Add the offset manually so the same
	ISR return code can be used in both cases. */
	__asm volatile ( "ADD		LR, LR, #4" );

	/* Perform the context switch. First save the context of the current
	task.*/
	portSAVE_CONTEXT();

	/* Find the highest priority task that is ready to run. */
	vTaskSwitchContext();

	__asm volatile( "clrex" );

	/* Restore the context of the new task. */
	portRESTORE_CONTEXT();
}

/*
 * FreeRTOS bottom-level IRQ vector handler
 */
void vFreeRTOS_IRQInterrupt ( void ) __attribute__((naked));
void vFreeRTOS_IRQInterrupt ( void )
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	ulCriticalNesting++;

	__asm volatile( "clrex" );

	/* Call the handler provided with the standalone BSP */
	__asm volatile( "bl  BT_ARCH_ARM_GIC_IRQHandler" );

	ulCriticalNesting--;

	/* Restore the context of the new task. */
	portRESTORE_CONTEXT();
}
