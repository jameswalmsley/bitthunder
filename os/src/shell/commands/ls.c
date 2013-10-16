#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_ls(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;
	char *szpDir = 0;
	BT_DIRENT oDirent;

	if(argc == 1) {
		szpDir = "/";
	} else if(argc == 2) {
		szpDir = argv[1];
	} else {
		bt_fprintf(hStdout, "Usage: %s [dir]\n", argv[0]);
		return -1;
	}

	BT_HANDLE hDir = BT_OpenDir(szpDir, &Error);
	if(!hDir) {
		bt_fprintf(hStdout, "ls: cannot access %s: No such file or directory\n", szpDir);
		return 0;
	}

	while (!BT_ReadDir(hDir, &oDirent))
	{
		BT_i8 attr[11] = "----------";
		if(oDirent.attr & BT_ATTR_DIR) {
			attr[0] = 'd';
		}

		bt_fprintf(hStdout, "%s %10d ", attr, oDirent.ullFileSize);
		bt_fprintf(hStdout, "%s\n", oDirent.szpName);
	}

	BT_CloseHandle(hDir);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "ls",
	.pfnCommand = bt_ls,
};
