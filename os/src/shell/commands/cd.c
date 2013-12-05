#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int bt_cd(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error = BT_ERR_NONE;

	if(argc != 1 && argc != 2) {
		bt_fprintf(hStdout, "Usage: %s {[path]}\n", argv[0]);
		return 0;
	}

	BT_i8 *relpath = NULL;

	const BT_i8 *path = argv[1];
	if(path[0] != '\\' && path[0] != '/') {	// Relative path, join with cwd.
		relpath = BT_kMalloc(BT_PATH_MAX);
		BT_GetCwd(relpath, BT_PATH_MAX);	// Kernel process paths never end in '/', its always removed.
		if(strlen(relpath) == 1) {			// Except for the root dir!
			strcat(relpath, path);
		} else {
			strcat(relpath, "/");
			strcat(relpath, path);
		}
	}

	BT_kPrint("changing dir to: %s", relpath ? relpath : path);

	Error = BT_ChDir(relpath ? relpath : path);
	if(Error) {
		bt_printf("%s: %s: No such file or directory\n", argv[0], relpath ? relpath : path);
	}

	if(relpath) {
		BT_kFree(relpath);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "cd",
	.pfnCommand = bt_cd,
};
