/**
 * BTShell - In-built help command.
 *
 **/

#include <bitthunder.h>
#include <shell/bt_shell.h>

extern const BT_SHELL_COMMAND * __bt_shell_commands_start;
extern const BT_SHELL_COMMAND * __bt_shell_commands_end;

static int bt_help_command(int argc, char **argv) {

	BT_u32 size = (BT_u32) ((BT_u32) &__bt_shell_commands_end - (BT_u32) &__bt_shell_commands_start);
	BT_u32 i;

	size /= sizeof(BT_SHELL_COMMAND);

	const BT_SHELL_COMMAND *pCommand = (BT_SHELL_COMMAND *) &__bt_shell_commands_start;

	bt_printf(BT_VERSION_STRING"\n");

	for(i = 0; i < size; i++) {
		bt_printf("%s\n", pCommand->szpName);
		pCommand++;
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "help",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_help_command,
};
