#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_free(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	struct bt_page_info oInfo;

	bt_page_info(&oInfo);

	BT_u32 i;
	bt_fprintf(hStdout, "               TOTAL        USED   USE\n");
	bt_fprintf(hStdout, "Normal  : %10d  %10d  %3d%%\n", oInfo.normal_size, oInfo.normal_used, (oInfo.normal_used * 100) / oInfo.normal_size);
	bt_fprintf(hStdout, "Coherent: %10d  %10d  %3d%%\n", oInfo.coherent_size, oInfo.coherent_used, (oInfo.coherent_used * 100) / oInfo.coherent_size);

	BT_u32 cached_total = 0, cached_used = 0;
	struct bt_slab_info slab_info;
	bt_slab_info(&slab_info);
	for(i = 0; i < BT_SLAB_MAX_ORDER; i++) {
		struct bt_cache_info *slab = &slab_info.slabs[i];
		cached_total += slab->available * slab->ulObjectSize;
		cached_used += slab->allocated * slab->ulObjectSize;
	}

	bt_fprintf(hStdout, "Cached  : %10d  %10d  %3d%%\n", cached_total, cached_used, (cached_used * 100) / cached_total);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "free",
	.pfnCommand = bt_free,
};
