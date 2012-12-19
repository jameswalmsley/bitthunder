#include <bitthunder.h>

extern BT_u32 BT_ZYNQ_GetArmPLLFrequency();


/**
 *	This will be running within a FreeRTOS Thread.
 *
 *	It is the boot/startup thread, and you can return from it after starting all your own processes.
 *	However, you can just use it as an already running process.
 **/

int main(int argc, char **argv) {

	int i;

	int y = BT_ZYNQ_GetArmPLLFrequency();

	while(1) {
		y += 1;
		i++;
		y -=1;
	}
}


