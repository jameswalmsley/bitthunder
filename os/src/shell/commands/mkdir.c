#include <bitthunder.h>


static int bt_mkdir(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error = BT_ERR_NONE;

	if(argc != 2) {
		bt_fprintf(hStdout, "Usage: %s [path]\n", argv[0]);
		return 0;
	}

	Error = BT_MkDir(argv[1]);

	return 0;
}

BT_SHELL_COMMAND_DEF oMkdirCommand = {
	.szpName 		= "mkdir",
	.pfnCommand 	= bt_mkdir,
};
