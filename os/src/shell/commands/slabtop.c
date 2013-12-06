#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static struct bt_slab_info slab_cached;
static BT_BOOL g_cached_valid = BT_FALSE;

static int bt_slabtop(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	struct bt_slab_info oInfo;

	bt_slab_info(&oInfo);

	BT_u32 i;
	bt_fprintf(hStdout, "OBJECTS (delta)    USED (delta)   USE   OBJSIZE  CACHESIZE\n");
	for(i = 0; i < BT_SLAB_MAX_ORDER; i++) {
		struct bt_cache_info *slab = &oInfo.slabs[i];
		BT_s32 d_allocated = 0, d_available = 0;
		if(g_cached_valid) {
			d_allocated = slab_cached.slabs[i].allocated - slab->allocated;
			d_available = slab_cached.slabs[i].available - slab->available;
		}

		bt_fprintf(hStdout, " %6d (%5d)  %6d (%5d)  %3d%%  %8d   %8d\n", slab->available, d_available, slab->allocated, d_allocated, (slab->allocated * 100) / slab->available, slab->ulObjectSize, slab->ulObjectSize * slab->available);
	}

	slab_cached = oInfo;
	g_cached_valid = BT_TRUE;

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "slabtop",
	.pfnCommand = bt_slabtop,
};
