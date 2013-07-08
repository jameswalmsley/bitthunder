#include <bitthunder.h>
#include <stdlib.h>

static int bt_load_command(int argc, char **argv) {

	BT_ERROR Error;

	if(argc != 3) {
		bt_printf("Usage: %s [address] [path]\n", argv[0]);
		return -1;
	}

	BT_HANDLE hFile = BT_Open(argv[2], "rb", &Error);
	if(!hFile) {
		bt_printf("Could not open path: %s\n", argv[2]);
		return -1;
	}

	BT_HANDLE hInode = BT_GetInode(argv[2], &Error);
	BT_INODE  oInode;

	BT_u32 addr;

	BT_ReadInode(hInode, &oInode);

	addr = strtoul(argv[1], NULL,  16);

	BT_kPrint("Loading %s at %08X (%llu bytes)", argv[2], addr, oInode.ullFilesize);

	void *p = (void *) addr;

	BT_Read(hFile, 0, oInode.ullFilesize, p, &Error);

	BT_kPrint("Load successful");

	if(hFile) {
		BT_CloseHandle(hFile);
	}

	if(hInode) {
		BT_CloseHandle(hInode);
	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "load",
	.eType = BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_load_command,
};
