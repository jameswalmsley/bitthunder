#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_echo(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_u32 i;

	for(i = 1; i < argc; i++) {
		bt_fprintf(hStdout, "%s ", argv[i]);
	}

	bt_fprintf(hStdout, "\n");

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "echo",
	.pfnCommand = bt_echo,
};
