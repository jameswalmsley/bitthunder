#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static void printmount(BT_HANDLE hStdout, BT_MOUNTPOINT *pMount) {
	if(!pMount) {
		return;
	}

	const BT_i8 *fs_name = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF->name;
	struct bt_fsinfo oInfo;

	oInfo.total = 0;
	oInfo.available = 0;

	BT_GetMountFSInfo(pMount, &oInfo);

	bt_fprintf(hStdout, "%-10s %8llu %8llu %8llu ",
			   fs_name,
			   oInfo.total,
			   oInfo.total-oInfo.available,
			   oInfo.available
		);

	bt_fprintf(hStdout, "%d %% %s\n", 1, pMount->szpPath);
}

static int bt_df(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_MOUNTPOINT *pMount = NULL;

	char *path = NULL;

	if(argc == 2) {
		path = argv[1];
	}

	if(path) {
		pMount = BT_GetMountPoint(path);
		printmount(hStdout, pMount);
	} else {
		while((pMount = BT_GetNextMountPoint(pMount)) != NULL) printmount(hStdout, pMount);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "df",
	.pfnCommand = bt_df,
};
