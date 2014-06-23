#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_date(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	struct bt_timeval tv;

	bt_gettimeofday(&tv, NULL);

	struct bt_tm tm;
	bt_time_to_tm(tv.tv_sec, &tm);

	bt_fprintf(hStdout, "%0d:%0d:%0d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);


	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "date",
	.pfnCommand = bt_date,
};
