#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int bt_exec(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	if(argc != 1 && argc != 2) {
		bt_fprintf(hStdout, "Usage: %s {[path]}\n", argv[0]);
		return 0;
	}

	BT_ExecImageFile(argv[1]);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "exec",
	.pfnCommand = bt_exec,
};
