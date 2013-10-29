#include <bitthunder.h>
#include <stdlib.h>

#define CALC_HASH	1

#if (CALC_HASH)

#include "md5.h"
#include "stdio.h"
#include "string.h"

#define CHUNKSZ_MD5 (64 * 1024)

static int calculate_hash (const void *data, int data_len, const char *algo, unsigned char *value, int *value_len)
{
	if (strcmp (algo, "md5") == 0 ) {
		md5_wd ((unsigned char *)data, data_len, value, CHUNKSZ_MD5);
		*value_len = 16;
	}

	return 0;
}

#endif

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


	BT_kPrint("Loading %s at %08X (%llu bytes)", argv[3], addr, oInode.ullFilesize);

	BT_Read(hFile, 0, oInode.ullFilesize, p, &Error);

	BT_kPrint("Load successful");

#if (CALC_HASH)

	unsigned char md5[16];
	char szmd5[33] = { 0 };
	int i, md5_len; 
	calculate_hash(p, oInode.ullFilesize, "md5", md5, &md5_len);	
	for(i=0;i<md5_len;i++) {
		char sztmp[3];
		sprintf(sztmp, "%02x", md5[i]);
		strcat(szmd5, sztmp);
	}
	BT_kPrint("MD5: %s", szmd5);

#endif

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
	.pfnCommand = bt_load_fpga,
};
