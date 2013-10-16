/**
 * BTShell - In-built help command.
 *
 **/

#include <bitthunder.h>
#include <shell/bt_shell.h>

extern const BT_SHELL_COMMAND * __bt_shell_commands_start;
extern const BT_SHELL_COMMAND * __bt_shell_commands_end;

static int bt_help_command(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	BT_u32 size = (BT_u32) ((BT_u32) &__bt_shell_commands_end - (BT_u32) &__bt_shell_commands_start);
	BT_u32 i;

	size /= sizeof(BT_SHELL_COMMAND);

	const BT_SHELL_COMMAND *pCommand = (BT_SHELL_COMMAND *) &__bt_shell_commands_start;

	bt_printf(BT_VERSION_STRING"\n");

	for(i = 0; i < size; i++) {
		bt_fprintf(hStdout, "%s\n", pCommand->szpName);
		pCommand++;
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "help",
	.pfnCommand = bt_help_command,
};
