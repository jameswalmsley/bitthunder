#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_ls(int argc, char **argv) {
	int ret = 0;
	BT_ERROR Error;
	char *szpDir = 0;
	BT_DIRENT oDirent;

	if(argc == 1) {
		szpDir = "/";
	} else if(argc == 2) {
		szpDir = argv[1];
	} else {
		bt_printf("Usage: %s [dir]\n", argv[0]);
		return -1;
	}

	BT_HANDLE hDir = BT_OpenDir(szpDir, &Error);

	while (!BT_ReadDir(hDir, &oDirent))
	{
		bt_printf("%s %d\n", oDirent.szpName, oDirent.ullFileSize);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "ls",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_ls,
};
