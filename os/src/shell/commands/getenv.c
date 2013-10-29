#include <bitthunder.h>
#include <shell/bt_env.h>
#include <stdlib.h>
#include <string.h>

 
static void printenv(BT_HANDLE hStdout, BT_ENV_VARIABLE *pEnv) {
	if(pEnv) {
		if(pEnv->eType == BT_ENV_T_STRING) {
			bt_fprintf(hStdout, "%s = %s\n", pEnv->s, pEnv->o.string->s);
		} else if(pEnv->eType == BT_ENV_T_INTEGER) {
			bt_fprintf(hStdout, "%s = %lu\n", pEnv->s, pEnv->o.integer->i);
		}
	}
}

static int bt_getenv(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	char *szpName = NULL;

	if(argc == 1) {
		// @@AF: print whole environment ...
	} else if(argc == 2) {
		szpName = argv[1];
	} else {
		bt_fprintf(hStdout, "Usage: %s [name]\n", argv[0]);
		return -1;
	}

	if(szpName) {
		BT_ENV_VARIABLE *pEnv = BT_ShellGetEnv(szpName);
		printenv(hStdout, pEnv);
	} else {
		BT_ENV_VARIABLE *pEnv = NULL;
		while((pEnv = BT_ShellGetNextEnv(pEnv)) != NULL) printenv(hStdout, pEnv);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "getenv",
	.pfnCommand = bt_getenv,
};

