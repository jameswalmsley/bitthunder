/*
 *	Raspberry Pi Porting layer for FreeRTOS.
 */

#include <bitthunder.h>
#include <bt_types.h>
#include <FreeRTOS.h>
#include <task.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

/* Constants required to setup the task context. */
#define portINITIAL_SPSR						( ( portSTACK_TYPE ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT						( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE					( ( portSTACK_TYPE ) 4 )
#define portNO_CRITICAL_SECTION_NESTING			( ( portSTACK_TYPE ) 0 )

#define portTIMER_PRESCALE 						( ( unsigned long ) 0xF9 )


static BT_ERROR tick_isr_handler(BT_u32 ulIRQ, void *pParam) {
	xTaskIncrementTick();

#if configUSE_PREEMPTION == 1
	vTaskSwitchContext();
#endif

	// Timer interrupt status is handled automatically by BitThunder driver.
	return BT_ERR_NONE;
}


/* Constants required to setup the VIC for the tick ISR. */
#define portTIMER_BASE                    		( (unsigned long ) 0x2000B400 )

typedef struct _BCM2835_TIMER_REGS {
	unsigned long LOD;
	unsigned long VAL;
	unsigned long CTL;
	unsigned long CLI;
	unsigned long RIS;
	unsigned long MIS;
	unsigned long RLD;
	unsigned long DIV;
	unsigned long CNT;
} BCM2835_TIMER_REGS;

static volatile BCM2835_TIMER_REGS * const pRegs = (BCM2835_TIMER_REGS *) (portTIMER_BASE);

/*-----------------------------------------------------------*/

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/*
 * The scheduler can only be started from ARM mode, so
 * vPortISRStartFirstSTask() is defined in portISR.c.
 */
extern void vPortISRStartFirstTask( void );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
portSTACK_TYPE *pxOriginalTOS;

	pxOriginalTOS = pxTopOfStack;

	/* To ensure asserts in tasks.c don't fail, although in this case the assert
	is not really required. */
	pxTopOfStack--;

	/* Setup the initial stack of the task.  The stack is set exactly as
	expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	start of the task.  The offset is added to make the return address appear
	as it would within an IRQ ISR. */
	*pxTopOfStack = ( portSTACK_TYPE ) pxCode + portINSTRUCTION_SIZE;
	pxTopOfStack--;

	*pxTopOfStack = ( portSTACK_TYPE ) 0xaaaaaaaa;	/* R14 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
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

	/* When the task starts it will expect to find the function parameter in
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
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	prvSetupTimerInterrupt();

	/* Start the first task. */
	vPortISRStartFirstTask();

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the ARM port will require this function as there
	is nothing to return to.  */
}
/*-----------------------------------------------------------*/

/*
 * Setup the timer 0 to generate the tick interrupts at the required frequency.
 */
static void prvSetupTimerInterrupt( void )
{
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
/*-----------------------------------------------------------*/

