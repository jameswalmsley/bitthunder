.extern	system_init
.extern __bss_start
.extern __bss_end
.extern BT_IRQHandler
.extern kBottomISR
.extern vTickISR
.extern BT_DisableInterrupts
.extern main
	.section .init
	.globl reset
;; 
__reset:
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
undefined_handler:  .word undefined
swi_handler:        .word undefined @vPortYieldProcessor
prefetch_handler:   .word prefetch
data_handler:       .word data
unused_handler:     .word unused
irq_handler:        .word undefined @kBottomISR
fiq_handler:        .word fiq

reset:
	;@	In the reset handler, we need to copy our interrupt vector table to 0x0000, its currently at 0x8000

	mov r0,#0x8000								;@ Store the source pointer
    mov r1,#0x0000								;@ Store the destination pointer.

	;@	Here we copy the branching instructions
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}			;@ Load multiple values from indexed address. 		; Auto-increment R0
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}			;@ Store multiple values from the indexed address.	; Auto-increment R1

	;@	So the branches get the correct address we also need to copy our vector table!
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}			;@ Load from 4*n of regs (8) as R0 is now incremented.
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}			;@ Store this extra set of data.


	;@	Set up the various STACK pointers for different CPU modes
    ;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD2
    msr cpsr_c,r0
    mov sp,#0x8000

    ;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD1
    msr cpsr_c,r0
    mov sp,#0x4000

    ;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD3
    msr cpsr_c,r0
	mov sp,#0x8000000

	ldr r0, =__bss_start
	ldr r1, =__bss_end

	mov r2, #0

zero_loop:
	cmp 	r0,r1
	it		lt
	strlt	r2,[r0], #4
	blt		zero_loop

	@bl BT_DisableInterrupts
	
	;@	Here we could copy BSS data to the correct locations, but we can also have a BL to system_init function
	bl system_init
	
	;@ 	mov	sp,#0x1000000
	b main									;@ We're ready?? Lets start main execution!
	.section .text

undefined:
	mov		r1, r14
	mov		r0, #10
	push	{r14}
	pop		{r14}
	b 		undefined

prefetch:
	mov		r1, r14
	mov		r0, #9
	push	{r14}
	pop		{r14}
	b 		undefined

data:
	mov		r1, r14
	mov		r0, #8
	push	{r14}
	pop		{r14}
	b data

unused:
	mov		r1, r14
	mov		r0, #7
	push	{r14}
	pop		{r14}
	b unused

fiq:
	mov		r1, r14
	mov		r0, #6
	push	{r14}
	pop		{r14}
	b 		fiq
	
hang:
	mov		r1,r14
	# Copy the exception return address to r1 ;@

	mov r0, r1	
	#bl led_loop
	
	b hang

swi:
	@b		vPortYieldProcessor

;@irq:
;@ 	push 	{r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
;@    bl 		BT_irqHandler
;@    pop  	{r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
;@    subs 	pc,lr,#4

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
	
