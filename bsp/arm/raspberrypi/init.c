#include <bitthunder.h>

//
//extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );
extern void enable_irq ( void );
extern void enable_fiq ( void );


typedef struct {
	BT_u32	GPFSEL[6];	///< Function selection registers.
	BT_u32	Reserved_1;
	BT_u32	GPSET[2];
	BT_u32	Reserved_2;
	BT_u32	GPCLR[2];
	BT_u32	Reserved_3;
	BT_u32	GPLEV[2];
	BT_u32	Reserved_4;
	BT_u32	GPEDS[2];
	BT_u32	Reserved_5;
	// etc etc
} BCM2835_GPIO_REGS;

volatile BCM2835_GPIO_REGS * const pRegs = (BCM2835_GPIO_REGS *) (0x20200000);


#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_CLI 0x2000B40C
#define ARM_TIMER_RIS 0x2000B410
#define ARM_TIMER_MIS 0x2000B414
#define ARM_TIMER_RLD 0x2000B418
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420

#define SYSTIMERCLO 0x20003004
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028

#define IRQ_BASIC 0x2000B200
#define IRQ_PEND1 0x2000B204
#define IRQ_PEND2 0x2000B208
#define IRQ_FIQ_CONTROL 0x2000B210
#define IRQ_ENABLE_BASIC 0x2000B218
#define IRQ_DISABLE_BASIC 0x2000B224


unsigned long *GetGpioAddress() {
	return (unsigned long *) 0x20200000;
}

void SetGpioFunction(unsigned int pinNum, unsigned char funcNum) {

	int offset = pinNum / 10;

	BT_u32 val = pRegs->GPFSEL[offset];
	int item = pinNum % 10;
	val &= ~(0x7 << (item * 3));
	val |= ((funcNum & 0x7) << (item * 3));
	pRegs->GPFSEL[offset] = val;
}

void SetGpio(int pinNum, int pinVal) {
	int offset = pinNum / 32;

	if(pinVal) {
		pRegs->GPSET[offset] = 1 << (pinNum % 32);		
	} else {
		pRegs->GPCLR[offset] = 1 << (pinNum % 32);
	}
}

int sleep(int cycles) {
	//register int i = 0x3f0000 >> 2;
	while(cycles--) {
		asm("");
	}
	return cycles;
}



volatile unsigned int icount;
void c_irq_handler();

extern void BT_InitInterruptController();


BT_ERROR task1(BT_HANDLE hProcess, void *pParam) {

	int i = 0;
	while(1) {
		i++;
		SetGpio(16, 1);
		//BT_ThreadDelay(200);
		//BT_ThreadYield();
	}

	return BT_ERR_NONE;
}

BT_ERROR task2(BT_HANDLE hProcess, void *pParam) {

	int i = 0;
	while(1) {
		i++;
		//BT_ThreadDelay(100);
		SetGpio(16, 0);
		//BT_ThreadDelay(100);
		//BT_ThreadYield();
	}

	return BT_ERR_NONE;
}



BT_ERROR timer0_irq_handler(void *pParam) {
	if(icount++ & 1) {
		SetGpioFunction(16, 1);			// RDY led
	} else {
		SetGpioFunction(16, 0);			// RDY led
	}

	PUT32(ARM_TIMER_CLI,0);
}


void system_init(void) {
	BT_ERROR 			Error;
	//BT_THREAD_CONFIG 	oConfig;

	//BT_DisableInterrupts();
	//BT_InitInterruptController();

	SetGpioFunction(16, 1);			// RDY led

	/*BT_InitInterruptController();
	BT_DisableInterrupt(64);

	BT_RegisterInterrupt(64, vTickISR, NULL);	
	
	PUT32(ARM_TIMER_CTL,0x003E0000);
	PUT32(ARM_TIMER_LOD,1000-1);
    PUT32(ARM_TIMER_RLD,1000-1);
	PUT32(ARM_TIMER_DIV,0x000000F9);
	PUT32(ARM_TIMER_CLI,0);
	PUT32(ARM_TIMER_CTL,0x003E00A2);

	BT_EnableInterrupt(64);*/

	icount = 0;

	/*oConfig.ulStackSize = 256;
  	oConfig.ulPriority = 0;
  	oConfig.bStartSuspended = BT_FALSE;
  	oConfig.bAutoRestart = BT_TRUE;
  	oConfig.pParam = NULL;

	BT_CreateProcess(task1, "Task1", &oConfig, &Error);
	BT_CreateProcess(task2, "Task2", &oConfig, &Error);*/


	//BT_EnableInterrupts();

	//led_loop();
	//vTaskStartScheduler();

	while(1) {;}
}




void main(void) {
//	SetGpioFunction(16, 1);3
	//oldSetGpioFunction(16, 1);
 
	while(1) {

		sleep(0x3f0000);

		sleep(0x3f0000);

		//oldSetGpio(16, 0);

	}

	return;
}




void c_irq_handler() {

    icount++;
    if(icount&1)
    {
        PUT32(GPCLR0,1<<16);
    }
    else
    {
        PUT32(GPSET0,1<<16);
	}
    PUT32(ARM_TIMER_CLI,0);
}
