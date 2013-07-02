#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

typedef void (*jump) 		(void);
typedef void (*jump_regs)	(BT_u32 a, BT_u32 b, BT_u32 c, BT_u32 d);

typedef struct _BOOT_PARAMS {
	jump 	jmp;				///< Application entry point.
#ifdef BT_CONFIG_SHELL_CMD_ATAGS
	BT_u32	atag_addr;
#endif
	BT_u32	flags;
	#define BOOT_FLAG_MACHID	0x00000001
	#define	BOOT_FLAG_ATAG		0x00000008
} BOOT_PARAMS;

static BOOT_PARAMS oBootParams[BT_CONFIG_CPU_CORES];

static void boot_core(BT_u32 coreID) {
	register BT_u32 a, b, c;

	__asm volatile("ldr sp,=0x30000");

	coreID = BT_GetCoreID();

	BT_kPrint("CoreID: %d", coreID);

	a = 0;
	b = 0;
	c = oBootParams[coreID].atag_addr;

	jump_regs jmp = (jump_regs) oBootParams[coreID].jmp;
	jmp(a, b, c, 0);
}

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
		coreID = strtol(argv[2], NULL, 10);
		addr = strtol(argv[3], NULL, 16);
	} else {
		addr = strtol(argv[1], NULL, 16);
	}
	void *p = (void *) addr;

	BT_DCacheFlush();
	BT_ICacheInvalidate();

	BT_DCacheDisable();

	if(!coreID) {
		BT_StopSystemTimer();
		BT_DisableInterrupts();

		oBootParams[0].jmp = (jump) p;
		boot_core(0);

		while(1) {
			__asm__ ("");
		}

	} else {
		// Must use MACH core boot interface.
		oBootParams[coreID].jmp	 		= (jump) p;
		BT_BootCore(coreID, boot_core);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "boot",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_boot,
};

#ifdef BT_CONFIG_SHELL_CMD_ATAGS
static int bt_boot_atag(int argc, char **argv) {

	if(argc != 3) {
		bt_printf("%s [coreID] [0x{atag_address}]\n");
		return -1;
	}

	BT_u32 coreID 	= strtol(argv[1], NULL, 10);
	BT_u32 addr		= strtol(argv[2], NULL, 16);

	oBootParams[coreID].flags 		|= BOOT_FLAG_ATAG;
	oBootParams[coreID].atag_addr 	 = addr;

	return 0;
}

BT_SHELL_COMMAND_DEF oAtagCommand = {
	.szpName	= "boot_atag",
	.eType		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_boot_atag,
};
#endif
