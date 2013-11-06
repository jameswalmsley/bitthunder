#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static void usage(BT_HANDLE hStdout, char *argv0) {
	bt_fprintf(hStdout, "Usage: %s [source] [target] {-t [filesystem]} \n", argv0);
	bt_fprintf(hStdout, "       eg. %s /dev/mmc00 /sd0/ -t vfat\n", argv0);
}

static int bt_mount(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;
	char *szpBlockDevice = 0;
	char *szpMountPoint = 0;
	char *szpFileSystem = 0;

	if(argc == 1) {
		// @@AF: list available mount points ... TODO
	} else if(argc == 3) {
		szpBlockDevice = argv[1];
		szpMountPoint = argv[2];
	} else if(argc == 5) {
		szpBlockDevice = argv[1];
		szpMountPoint = argv[2];
		if(!strcmp(argv[3],"-t")) {
			szpFileSystem = argv[4];
		} else {
			usage(hStdout, argv[0]);
			return -1;
		}
	} else {
		usage(hStdout, argv[0]);
		return -1;
	}

	Error = BT_Mount(szpBlockDevice, szpMountPoint, szpFileSystem, 0, NULL);
	if(Error) {
		return -1;
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "mount",
	.pfnCommand = bt_mount,
};
