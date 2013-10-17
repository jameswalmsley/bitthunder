#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_mount(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;
	char *szpBlockDevice = 0;
	char *szpMountPoint = 0;

	if(argc == 1) {
		// @@AF: list available mount points ... TODO
	} else if(argc == 3) {
		szpBlockDevice = argv[1];
		szpMountPoint = argv[2];
	} else {
		bt_fprintf(hStdout, "Usage: %s [dev] [dir]\n", argv[0]);
		return -1;
	}

	BT_HANDLE hVolume = BT_DeviceOpen(szpBlockDevice, &Error);
	if(!hVolume) {
		return -1;
	}

	Error = BT_Mount(hVolume, szpMountPoint);
	if(Error) {
		return -1;
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "mount",
	.pfnCommand = bt_mount,
};
