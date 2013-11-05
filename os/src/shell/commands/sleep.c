#include <bitthunder.h>
#include <stdlib.h>

struct process_time {
	BT_u32 ullRuntimeCounter;
};

static int bt_sleep(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	if(argc != 2) {
		bt_fprintf(hStdout, "Usage: %s [seconds]", argv[0]);
		return -1;
	}

	BT_u32 seconds = strtoul(argv[1], NULL, 10);

	BT_ThreadSleep(1000 * seconds);

	return 0;
}


BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "sleep",
	.pfnCommand = bt_sleep,
};
