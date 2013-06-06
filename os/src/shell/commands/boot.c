#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

typedef void (*jump) (void);

static int bt_boot(int argc, char **argv) {

	if(argc != 2 && argc != 4) {
		bt_printf("Usage: %s {--core [coreID]} [start-address]\n", argv[0]);
		return -1;
	}

	BT_u32 coreID = 0;
	BT_u32 addr;

	if(argc == 4) {
		if(!strcmp(argv[1], "--core")) {
			bt_printf("Invalid argument %s\n", argv[1]);
			return -1;
		}
		coreID = strtol(argv[2], NULL, 10);
		addr = strtol(argv[3], NULL, 16);
	} else {
		addr = strtol(argv[1], NULL, 16);
	}
	void *p = (void *) addr;

	BT_DCacheFlush();
	BT_ICacheInvalidate();

	if(!coreID) {
		BT_StopSystemTimer();
		BT_DisableInterrupts();

		jump jmp = p;
		jmp();

		while(1) {
			__asm__ ("");
		}

	} else {
		// Must use MACH core boot interface.
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "boot",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_boot,
};
