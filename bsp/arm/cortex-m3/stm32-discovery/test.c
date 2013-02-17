#include <bitthunder.h>
#include <string.h>


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

int main(int argc, char **argv) {

	BT_GpioSetDirection(41, BT_GPIO_DIR_OUTPUT);

	while(1) {
		BT_GpioSet(41, BT_FALSE);
		BT_kTaskDelay(500);
		BT_GpioSet(41, BT_TRUE);
		BT_kTaskDelay(500);
	}

	return 0;
}

