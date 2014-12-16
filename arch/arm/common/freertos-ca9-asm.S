#include <bt_config.h>

.equ SYS_MODE,			0x1f
.equ SVC_MODE,			0x13
.equ IRQ_MODE,			0x12

.macro portSAVE_CONTEXT
	// Save the LR and SPSR onto the system mode stack before switching to
	// system mode to save the remaining system mode registers
	SRSDB	sp!, #SYS_MODE
	CPS		#SYS_MODE
	PUSH	{R0-R12, R14}

	// Push the critical nesting count
	LDR		R2, ulCriticalNestingConst
	LDR		R1, [R2]
	PUSH	{R1}

#ifdef BT_CONFIG_TOOLCHAIN_FLOAT_HARD
	// Save the floating point context
	VPUSH 	{D0-D15}
	VPUSH	{D16-D31}
	VMRS	R0, FPSCR
	VMRS	R1, FPEXC
	PUSH	{R0-R1}
#endif

	// Save the stack pointer in the TCB
	LDR		R0, pxCurrentTCBConst
	LDR		R1, [R0]
	STR		SP, [R1]
.endm

.macro portRESTORE_CONTEXT
	// Set the SP to point to the stack of the task being restored.
	LDR		R0, pxCurrentTCBConst
	LDR		R1, [R0]
	LDR		SP, [R1]

#ifdef BT_CONFIG_TOOLCHAIN_FLOAT_HARD
	// Restore the floating point context.
	POP		{R0-R1}
	VMSR	FPEXC, r1
	VMSR	FPSCR, r0
	VPOP	{D16-D31}
	VPOP	{D0-D15}
#endif

	// Restore the critical section nesting depth
	LDR		R0, ulCriticalNestingConst
	POP		{R1}
	STR		R1, [R0]

	// Restore all system mode registers other than the SP (which is already
	// being used)
	POP		{R0-R12, R14}

	// Return to the task code, loading CPSR on the way.
	RFEIA	sp!
.endm

.globl vFreeRTOS_SWI_Handler
	portSAVE_CONTEXT
	LDR	 	r0, =vTaskSwitchContext
	BLX		r0

.globl vPortRestoreTaskContext
vPortRestoreTaskContext:
	portRESTORE_CONTEXT

.globl vFreeRTOS_IRQInterrupt
vFreeRTOS_IRQInterrupt:
	// Return to the interrupted instruction
	SUB		lr, lr, #4

	//  Push the return address and SPSR
	PUSH	{lr}
	MRS		lr, SPSR
	PUSH	{lr}

	// Change to supervisor mode to allow reentry.
	//CPS		#SVC_MODE

	// Push used regs
	PUSH	{r0-r4, r12}

	// Increment the nesting counts
	LDR		r3, ulPortInterruptNestingConst
	LDR		r1, [r3]
	ADD		r4, r1, #1
	STR		r4, [r3]

	// Ensure bit 2 of SP is cleared.
	MOV		r2, sp
	AND		r2, r2, #4
	SUB		sp, sp, r2

	// Call the interrupt handler
	PUSH	{r0-r3, lr}
	LDR 	r1, vApplicationIRQHandlerConst
	BLX 	r1
	POP		{r0-r3, lr}
	ADD		SP, SP, r2

	//CPSID	i
	//dsb
	//isb

	// Restore the nesting count
	STR		r1, [r3]

	CMP		r1, #0
	BNE		exit_without_switch

	LDR		r1, =ulPortYieldRequired
	LDR		r0, [r1]
	cmp		r0, #0
	bne		switch_before_exit

exit_without_switch:
	// No context switch, Restore used registers, LR_irq and SPSR before returning
	POP		{r0-r4, r12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf,	LR
	POP		{LR}
	MOVS	PC, LR

switch_before_exit:
	MOV		R0, #0
	STR		R0, [r1]

	// Restore the used registers, LR-irq and SPSR before saving the context
	// to the task stack.

	POP		{R0-R4, R12}
	CPS		#IRQ_MODE
	POP		{LR}
	MSR		SPSR_cxsf,	LR
	POP		{LR}
	portSAVE_CONTEXT

	// Call the function that selects the new task to execute.
	// vTaskSwitchContext() if vTaskSwitchContext() uses LDRD or STRD
	// instructions, or 8 byte aligned stack allocated data.  LR does not need
	// saving as a new LR will be loaded by portRESTORE_CONTEXT anyway.
	LDR 	R0, vTaskSwitchContextConst
	BLX 	R0

	// Restore the context of, and branch to, the task selected to execute next.
	portRESTORE_CONTEXT


.globl vPortYieldProcessor
vPortYieldProcessor:
	portSAVE_CONTEXT
	BL		vTaskSwitchContext
	portRESTORE_CONTEXT


pxCurrentTCBConst: .word pxCurrentTCB
ulCriticalNestingConst: .word ulCriticalNesting
vTaskSwitchContextConst: .word vTaskSwitchContext
ulPortInterruptNestingConst: .word ulPortInterruptNesting
vApplicationIRQHandlerConst: .word BT_ARCH_ARM_GIC_IRQHandler
