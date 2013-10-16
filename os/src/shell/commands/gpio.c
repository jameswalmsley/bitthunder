#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

// gpio [nr] [state]

static int bt_gpio(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	if(argc != 3) {
		bt_fprintf(hStdout, "Usage: %s {gpio_nr} {1|0}\n", argv[0]);
		return -1;
	}

	BT_u32 gpio 	= strtoul(argv[1], NULL, 10);
	BT_u32 state 	= strtoul(argv[2], NULL, 10);

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
	.pfnCommand = bt_gpio,
};

static int bt_gpio_dir(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	if(argc != 3) {
		bt_fprintf(hStdout, "Usage: %s {gpio_nr} {1|0}\n", argv[0]);
		return -1;
	}

	BT_u32 gpio 	= strtoul(argv[1], NULL, 10);
	BT_u32 state 	= strtoul(argv[2], NULL, 10);

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
	.pfnCommand = bt_gpio_dir,
};
