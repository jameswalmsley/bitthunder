#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

// gpio [nr] [state]

static int bt_gpio(int argc, char **argv) {

	if(argc != 3) {
		bt_printf("Usage: %s {gpio_nr} {1|0}\n", argv[0]);
		return -1;
	}

	BT_u32 gpio 	= strtol(argv[1], NULL, 10);
	BT_u32 state 	= strtol(argv[2], NULL, 10);

	if(!state) {
		state = BT_FALSE;
	} else {
		state = BT_TRUE;
	}

	BT_GpioSet(gpio, state);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "gpio",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_gpio,
};

static int bt_gpio_dir(int argc, char **argv) {

	if(argc != 3) {
		bt_printf("Usage: %s {gpio_nr} {1|0}\n", argv[0]);
		return -1;
	}

	BT_u32 gpio 	= strtol(argv[1], NULL, 10);
	BT_u32 state 	= strtol(argv[2], NULL, 10);

	if(!state) {
		state = BT_GPIO_DIR_INPUT;
	} else {
		state = BT_GPIO_DIR_OUTPUT;
	}

	BT_GpioSetDirection(gpio, state);

	return 0;
}

BT_SHELL_COMMAND_DEF oDirCommand = {
	.szpName 	= "gpio_dir",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_gpio_dir,
};
