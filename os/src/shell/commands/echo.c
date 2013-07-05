#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_echo(int argc, char **argv) {
	BT_u32 i;

	for(i = 1; i < argc; i++) {
		bt_printf("%s ", argv[i]);
	}

	bt_printf("\n");

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "echo",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_echo,
};
