#include <bitthunder.h>

static int bt_source(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	if(argc != 2) {
		bt_fprintf(hStdout, "Usage: %s [path]\n", argv[0]);
		return -1;
	}

	return BT_ShellScript(hShell, argv[1]);
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "source",
	.pfnCommand = bt_source,
};
