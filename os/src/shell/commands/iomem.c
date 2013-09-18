#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

typedef enum _MODE {
	MODE_UNKNOWN=0,
	MODE_R32,
	MODE_W32,
} MODE;

struct _MODE_STRING {
	BT_i8  *mode_string;
	MODE	eMode;
};

struct _MODE_STRING g_modes[] = {
	{"r32", MODE_R32},
	{"w32", MODE_W32},
};

static MODE getmode(BT_i8 *string) {
	BT_u32 i;
	for(i = 0; i < (sizeof(g_modes)/sizeof(struct _MODE_STRING)); i++) {
		if(!strcmp(g_modes[i].mode_string, string)) {
			return g_modes[i].eMode;
		}
	}

	return MODE_UNKNOWN;
}

static int usage(char **argv) {
	bt_printf("Usage: %s <mode> <addr>\n", argv[0]);
	return -1;
}

static void print_addr(BT_u32 *p) {
	bt_printf("0x%08x : 0x%08x\n", (BT_u32 ) p, *p);
}

static int bt_iomem(int argc, char **argv) {
	if(argc != 3 && argc != 4) {
		return usage(argv);
	}

	BT_u32 addr = strtoul(argv[2], NULL, 16);
	BT_u32 *p = (BT_u32 *) addr;

	MODE mode = getmode(argv[1]);
	switch(mode) {
	case MODE_R32:
		print_addr(p);
		break;

	case MODE_W32:
		if(argc != 4) {
			return usage(argv);
		}

		BT_u32 value = strtoul(argv[3], NULL, 16);
		*p = value;
		print_addr(p);
		break;

	default:
		break;
	}

	return 0;
}


BT_SHELL_COMMAND_DEF oIomemCommand = {
	.szpName 	= "iomem",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_iomem,
};
