#include <bitthunder.h>
#include <stdlib.h>

static int bt_memcat(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;

	if(argc != 4) {
		bt_fprintf(hStdout, "Usage: %s [start_address] [length] [path]\n", argv[0]);
		return -1;
	}

	BT_u32 length = 0;
	BT_u32 addr = strtoul(argv[1], NULL, 16);
	void *p = (void *) addr;

	length = strtoul(argv[2], NULL, 10);

	BT_HANDLE hFile = BT_Open(argv[3], "wb", &Error);
	if(!hFile) {
		bt_fprintf(hStdout, "Could not open %s for writing\n", argv[3]);
		return -1;
	}

	BT_Write(hFile, 0, length, p, &Error);

	BT_CloseHandle(hFile);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "memcat",
	.pfnCommand = bt_memcat,
};
