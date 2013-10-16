#include <bitthunder.h>
#include <stdlib.h>

static int bt_load_fpga(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);
	BT_ERROR Error;

	if(argc != 4 && argc != 5) {
		bt_fprintf(hStdout, "Usage: %s [buffer_address] [fpga_device] [bitstream] [length]\n", argv[0]);
		return -1;
	}

	BT_u32 length = 0;
	BT_u32 addr = strtoul(argv[1], NULL, 16);
	void *p = (void *) addr;

	if(argv[3][0] == '-' && argc == 5) {
		// Image loaded and length provided.
		length = strtoul(argv[4], NULL, 10);
		goto flush;
	}

	BT_HANDLE hFile = BT_Open(argv[3], "rb", &Error);
	if(!hFile) {
		bt_fprintf(hStdout, "Could not open bitstream at %s\n", argv[3]);
		return -1;
	}

	BT_HANDLE hInode = BT_GetInode(argv[3], &Error);
	if(!hInode) {
		bt_fprintf(hStdout, "Could not stat bitstream at %s\n", argv[3]);
		BT_CloseHandle(hFile);
		return -1;
	}

	BT_INODE oInode;
	BT_ReadInode(hInode, &oInode);

	BT_u32 addr = strtoul(argv[1], NULL, 16);

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

	length = oInode.ullFilesize;

flush:

	BT_DCacheFlush();

	BT_HANDLE hFPGA = BT_DeviceOpen(argv[2], &Error);
	if(!hFPGA) {
		bt_printf("Failed to open fpga device %s\n", argv[2]);
		return -1;
	}

	BT_Write(hFPGA, 0, length, p, &Error);

	BT_CloseHandle(hFPGA);

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName 	= "load_fpga",
	.eType   	= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand = bt_load_fpga,
};
