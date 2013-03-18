/**
 * BTShell - In-built help command.
 *
 **/

#include <bitthunder.h>
#include <shell/bt_shell.h>

static int bt_help_command(int argc, char **argv) {

	bt_printf("BT Shell (help):\n");

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "help",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_help_command,
};
