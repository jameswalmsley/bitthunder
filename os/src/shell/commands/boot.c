#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

typedef void (*jump) 		(void);
typedef void (*jump_regs)	(BT_u32 a, BT_u32 b, BT_u32 c, BT_u32 d);

static int bt_boot(int argc, char **argv) {

	if(argc != 2 && argc != 4) {
		bt_printf("Usage: %s {--core [coreID]} [start-address]\n", argv[0]);
		return -1;
	}

	BT_u32 coreID = 0;
	BT_u32 addr;

	if(argc == 4) {
		if(strcmp(argv[1], "--core")) {
			bt_printf("Invalid argument %s\n", argv[1]);
			return -1;
		}
		coreID = strtoul(argv[2], NULL, 10);
		addr = strtoul(argv[3], NULL, 16);
	} else {
		addr = strtoul(argv[1], NULL, 16);
	}
	void *p = (void *) addr;

	BT_DCacheFlush();
	BT_ICacheInvalidate();

	if(BT_GetCoreID() == coreID) {
		BT_StopSystemTimer();
		BT_DisableInterrupts();

		jump_regs jumpr = (jump_regs) p;
		jumpr(0, 0, 0, 0);

		while(1) {
			__asm__ ("");
		}

	} else {
		// Must use MACH core boot interface.
		BT_BootCore(coreID, p, 0, 0, 0, 0);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "boot",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_boot,
};
