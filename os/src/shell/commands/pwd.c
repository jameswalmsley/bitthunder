#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_pwd(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	BT_i8 *cwd = BT_kMalloc(BT_PATH_MAX);
	BT_GetCwd(cwd, BT_PATH_MAX);

	bt_fprintf(hStdout, "%s\n", cwd);

	BT_kFree(cwd);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "pwd",
	.pfnCommand = bt_pwd,
};
