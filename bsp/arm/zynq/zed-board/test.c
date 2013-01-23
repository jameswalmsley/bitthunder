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

#define BUF_SIZE	(1024*64)

BT_u32 buffer_a[BUF_SIZE/4];
BT_u32 *buffer_b = 0x01100000;

//BT_u32 buffer_a[1024*4];
//BT_u32 buffer_b[1024*4];

int main(int argc, char **argv) {

	BT_GpioSetDirection(0, BT_GPIO_DIR_OUTPUT);
	BT_GpioSetDirection(7, BT_GPIO_DIR_OUTPUT);

	//BT_GpioEnableInterrupt(0);

	BT_u32 start, end;

	buffer_a[0] = buffer_b[0];
	BT_u32 i;
	start = BT_GetSystemTimerOffset();
	for(i = 0; i  < 1024; i++) {
		buffer_a[i] = buffer_b[i];
	}
	end = BT_GetSystemTimerOffset();

	start = BT_GetKernelTime();
	for(i=0; i < 1024; i++) {
		memcpy(buffer_a, buffer_b, BUF_SIZE);
	}
	end = BT_GetKernelTime();

	while(1) {
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
		BT_GpioSet(0, BT_TRUE);
		BT_GpioSet(0, BT_FALSE);
	}

	return 0;
}
