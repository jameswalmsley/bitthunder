#include <bitthunder.h>

static int bt_source(int argc, char **argv) {

	if(argc != 2) {
		bt_printf("Usage: %s [path]\n", argv[0]);
		return -1;
	}

	return BT_ShellScript(argv[1]);
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "source",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_source,
};
