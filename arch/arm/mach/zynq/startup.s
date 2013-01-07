.extern	system_init
.extern __bss_start
.extern __bss_end
.extern main
.extern BT_ARCH_ARM_GIC_IRQHandler
.extern vPortYieldProcessor
.extern IRQInterruptHandler
.extern ps7_init
	.section .init
;@	.globl _btstart
.extern _boot

.globl reset
.globl _fini
.globl _init

.extern _vector_table

.globl _vector_table
_vector_table:
	;@ All the following instruction should be read as:
	;@ Load the address at symbol into the program counter.

	ldr	pc,reset_handler		;@ 	Processor Reset handler 		-- we will have to force this on the raspi!
	;@ Because this is the first instruction executed, of cause it causes an immediate branch into reset!

	ldr pc,undefined_handler	;@ 	Undefined instruction handler 	-- processors that don't have thumb can emulate thumb!
    ldr pc,swi_handler			;@ 	Software interrupt / TRAP (SVC) -- system SVC handler for switching to kernel mode.
    ldr pc,prefetch_handler		;@ 	Prefetch/abort handler.
    ldr pc,data_handler			;@ 	Data abort handler/
    ldr pc,unused_handler		;@ 	-- Historical from 26-bit addressing ARMs -- was invalid address handler.
    ldr pc,irq_handler			;@ 	IRQ handler
    ldr pc,fiq_handler			;@ 	Fast interrupt handler.

	;@ Here we create an exception address table! This means that reset/hang/irq can be absolute addresses
reset_handler:      .word reset
undefined_handler:  .word undef
swi_handler:        .word undef ;@vPortYieldProcessor
prefetch_handler:   .word prefetch
data_handler:       .word data
unused_handler:     .word hang
irq_handler:        .word BT_ARCH_ARM_GIC_IRQHandler ;@vFreeRTOS_IRQInterrupt
fiq_handler:        .word fiq

	.extern _stack
	.extern _init_begin
reset:

	ldr	sp, =_stack

	@ Enable Neon

	MRC	p15,0,r0,c1,c0,2		;

	bl 	_bt_startup_init_hook
	bl  _bt_startup_boot

@ 	;@ Copy the interrupt vector tables back to address 0x0
@
@ 	ldr	r0, =_init_begin
@ 	mov r1,#0x00000000
@
@ 	ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
@ 	stmia r1!,{r2-r9}
@
@ 	ldmia r0!,{r2-r9}
@ 	stmia r1!,{r2-r9}
@
@ 	/*mrs	r0, CPSR
@ 	orr	r0, r0, #0xC0			;@ Disable interrupts!
@ 	msr CPSR, r0*/
@
@ 	/*b reset*/
@
@
@
@ 	bl	ps7_init
@#	b	_boot
@ 	;@	In the reset handler, we need to copy our interrupt vector table to 0x0000, its currently at 0x8000
@
@
@ 	;@	Zero BSS section.
@ 	ldr	r0, =__bss_start
@ 	ldr	r1,	=__bss_end
@
@ 	mov	r2,	#0
@
@ 	bl	main

.globl _bt_startup_init_hook
.weak _bt_startup_init_hook
_bt_startup_init_hook:
	bx	lr

.globl _bt_startup_boot
.weak _bt_startup_boot
_bt_startup_boot:
	bx 	lr


zero_loop:
	cmp		r0, r1
	it		lt
	strlt	r2, [r0], #4
	blt		zero_loop



	;@;@	Set up the various STACK pointers for different CPU modes
    ;@;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    ;@mov r0,#0xD2
    ;@msr cpsr_c,r0
    ;@mov sp,#0x8000
	;@
    ;@;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    ;@mov r0,#0xD1
    ;@msr cpsr_c,r0
    ;@mov sp,#0x4000
	;@
    ;@;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    ;@mov r0,#0xD3
    ;@msr cpsr_c,r0
	;@mov sp,#0x8000000
	;@
	;@;@	Here we could copy BSS data to the correct locations, but we can also have a BL to system_init function
	;@;@bl system_init
	;@


	mov	sp,#0x00030000
	#b main				   @ We're ready?? Lets start main execution!
	.section .text

hang:
	b hang

undef:
	b undef

swi:
	b swi

prefetch:
	b prefetch

data:
	b data

fiq:
	b fiq

irq:
	push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
;@	     bl IRQInterruptHandler
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    subs pc,lr,#4

.globl	GET32
GET32:
	ldr r0,[r0]
	bx 	lr

.globl	PUT32
PUT32:
	str	r1,[r0]
	bx	lr

.globl enable_irq
enable_irq:
	mrs	r0,cpsr
	bic	r0,r0,#0x80
	msr	cpsr_c,r0
	bx	lr


_fini:
	bx lr

_init:
	bx lr

