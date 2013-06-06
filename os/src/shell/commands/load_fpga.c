#include <bitthunder.h>
#include <stdlib.h>

static int bt_load_fpga(int argc, char **argv) {

	BT_ERROR Error;

	if(argc != 4) {
		bt_printf("Usage: %s [buffer_address] [fpga_device] [bitstream]\n", argv[0]);
		return -1;
	}

	BT_HANDLE hFile = BT_Open(argv[3], "rb", &Error);
	if(!hFile) {
		bt_printf("Could not open bitstream at %s\n", argv[3]);
		return -1;
	}

	BT_HANDLE hInode = BT_GetInode(argv[3], &Error);
	if(!hInode) {
		bt_printf("Could not stat bitstream at %s\n", argv[3]);
		BT_CloseHandle(hFile);
		return -1;
	}

	BT_INODE oInode;
	BT_ReadInode(hInode, &oInode);

	BT_u32 addr = strtol(argv[1], NULL, 16);

	BT_kPrint("Loading %s at %08X (%llu bytes)", argv[3], addr, oInode.ullFilesize);

	void *p = (void *) addr;

	BT_Read(hFile, 0, oInode.ullFilesize, p, &Error);

	BT_kPrint("Load successful");

	if(hFile) {
		BT_CloseHandle(hFile);
	}

	if(hInode) {
		BT_CloseHandle(hInode);
	}

	BT_DCacheFlush();

	BT_HANDLE hFPGA = BT_DeviceOpen(argv[2], &Error);
	if(!hFPGA) {
		bt_printf("Failed to open fpga device %s\n", argv[2]);
		return -1;
	}

	BT_Write(hFPGA, 0, oInode.ullFilesize, p, &Error);

	BT_CloseHandle(hFPGA);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "load_fpga",
	.eType   	= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_load_fpga,
};
