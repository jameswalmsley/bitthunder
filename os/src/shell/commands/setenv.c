#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>


static int clear_env(char *name) {
	BT_ENV_VARIABLE *pEnv = BT_ShellGetEnv(name);
	if(!pEnv) {
		return 0;
	}

	strcpy(pEnv->o.string->s, "");

	return 0;
}


static int bt_setenv(int argc, char **argv) {

	BT_u32 total_length = 0;
	for(i = 2; i < argc; i++) {
		total_length += strlen(argv[i]) + 1;	// Add a space after each item.
	}

	if(!total_length) {
		clear_env(argv[1]);
		return 0;
	}

	char *s = BT_kMalloc(total_length);
	if(!s) {
		bt_printf("No memory could not allocated temporary memory to construct string\n");
		return -1;
	}

	strcpy(s, "");

	for(i = 2; i < argc; i++) {
		strcat(s, argv[i]);
		strcat(s, " ");
	}

	BT_ShellSetEnv(argv[1], s, BT_ENV_T_STRING);
	BT_kFree(s);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "setenv",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_setenv,
};
