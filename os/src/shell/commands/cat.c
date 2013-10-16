#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_cat(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;
	char *szpPath = 0;

	if(argc == 1) {
		szpPath = "/";
	} else if(argc == 2) {
		szpPath = argv[1];
	} else {
		bt_fprintf(hStdout, "Usage: %s [path]\n", argv[0]);
		return -1;
	}


	BT_HANDLE hFile = BT_Open(szpPath, "rb", &Error);
	if(!hFile) {
		bt_fprintf(hStdout, "cat: cannot access %s: No such file or not a file\n", szpPath);
		return 0;
	}

	BT_s32 c;
	while((c = BT_GetC(hFile, 0, &Error)) >= 0) {
		bt_fprintf(hStdout, "%c", c);
	}

	bt_fprintf(hStdout, "\n");

	BT_CloseHandle(hFile);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "cat",
	.pfnCommand = bt_cat,
};
