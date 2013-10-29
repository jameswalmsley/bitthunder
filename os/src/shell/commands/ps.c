#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

struct process_time {
	BT_u32 ullRuntimeCounter;
};

static int bt_ps(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_u32 i = 0;
	struct bt_process_time oTime;

	BT_u32 total_processes = BT_GetTotalProcesses();

	struct process_time *runtimes = BT_kMalloc(sizeof(struct process_time) * total_processes);
	if(!runtimes) {
		return -1;
	}


	BT_u64 runtime = BT_GetGlobalTimer();
	for(i = 0; i < total_processes; i++) {
		BT_GetProcessTime(&oTime, i);
		runtimes[i].ullRuntimeCounter = oTime.ullRunTimeCounter;
	}

	BT_ThreadSleep(1000);

	runtime = BT_GetGlobalTimer() - runtime;
	for(i = 0; i < total_processes; i++) {
		BT_GetProcessTime(&oTime, i);
		runtimes[i].ullRuntimeCounter = oTime.ullRunTimeCounter - runtimes[i].ullRuntimeCounter;
	}

	for(i = 0; i < total_processes; i++) {
		BT_GetProcessTime(&oTime, i);
		bt_fprintf(hStdout, "%s : %d%%\n", oTime.name, runtimes[i].ullRuntimeCounter / (runtime / 100));
	}

	bt_fprintf(hStdout, "Total runtime %d seconds\n", (BT_u32) (BT_GetGlobalTimer() / (BT_u64)BT_GetGlobalTimerRate()));

	BT_kFree(runtimes);

	return 0;
}


BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "ps",
	.pfnCommand = bt_ps,
};
