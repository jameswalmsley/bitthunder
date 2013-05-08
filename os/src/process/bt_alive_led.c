/**
 *	Provides an "I'm alive!" signal via a cool flashing LED sequence.
 *
 **/
#include <bitthunder.h>

BT_DEF_MODULE_NAME			("Alive Led")
BT_DEF_MODULE_DESCRIPTION	("Flashes an LED to indicate kernel is alive")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static void led_task(void *pParam) {

	BT_GpioSetDirection(BT_CONFIG_ALIVE_LED_GPIO, BT_GPIO_DIR_OUTPUT);

	BT_TICK ticks = BT_kTickCount();
	BT_TICK ticks_a = ticks;

	while(1) {
		ticks_a = BT_kTickCount();
		BT_GpioSet(BT_CONFIG_ALIVE_LED_GPIO, BT_TRUE);
		BT_kTaskDelayUntil(&ticks_a, 10);
		BT_GpioSet(BT_CONFIG_ALIVE_LED_GPIO, BT_FALSE);
		BT_kTaskDelayUntil(&ticks_a, 50);
		BT_GpioSet(BT_CONFIG_ALIVE_LED_GPIO, BT_TRUE);
		BT_kTaskDelayUntil(&ticks_a, 10);
		BT_GpioSet(BT_CONFIG_ALIVE_LED_GPIO, BT_FALSE);

		BT_kTaskDelayUntil(&ticks, BT_CONFIG_ALIVE_LED_PERIOD);
	}

	BT_kTaskDelete(NULL);
}


static BT_ERROR bt_led_init() {

	BT_ERROR Error = BT_ERR_NONE;

	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth 	= 128,
		.ulPriority		= BT_CONFIG_ALIVE_LED_PRIORITY,
	};

	BT_kTaskCreate((BT_FN_TASK_ENTRY) led_task, "LED", &oThreadConfig, &Error);

	return Error;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_led_init,
};
