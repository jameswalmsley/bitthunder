#include <bitthunder.h>

extern BT_u32 BT_ZYNQ_GetArmPLLFrequency();


/**
 *	This will be running within a FreeRTOS Thread, with lowest priority.
 *	It is the boot/startup thread, and you can return from it after starting all your own processes.
 *	Doing so will cause the thread to be killed. Note that any "process owned" resources will also
 *	be automatically destroyed.
 *
 *	Do not use this thread to pass resources to other processes.
 *
 *	However, you can just use it as an already running process.
 **/

static BT_ERROR gpioIRQHandler(BT_u32 ulIRQ, void *pParam) {


	return BT_ERR_NONE;
}

extern void test_gic(void);

int main(int argc, char **argv) {

	int i;

	int y = BT_ZYNQ_GetArmPLLFrequency();

	BT_GpioSetDirection(0, BT_GPIO_DIR_INPUT);

	BT_RegisterInterrupt(52, gpioIRQHandler, (void *) 1);
	BT_EnableInterrupt(52);
	BT_GpioEnableInterrupt(0);

	while(1) {
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);

		test_gic();
	}

	while(1) {
		y += 1;
		i++;
		y -=1;
	}

	return 0;
}
