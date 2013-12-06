#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_slabtop(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	struct bt_slab_info oInfo;

	bt_slab_info(&oInfo);

	BT_u32 i;
	bt_fprintf(hStdout, "OBJECTS    USED   USE   OBJSIZE  CACHESIZE\n");
	for(i = 0; i < BT_SLAB_MAX_ORDER; i++) {
		struct bt_cache_info *slab = &oInfo.slabs[i];
		bt_fprintf(hStdout, " %6d  %6d  %3d%%  %8d   %8d\n", slab->available, slab->allocated, (slab->allocated * 100) / slab->available, slab->ulObjectSize, slab->ulObjectSize * slab->available);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "slabtop",
	.pfnCommand = bt_slabtop,
};
