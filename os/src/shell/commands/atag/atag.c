/**
 *	Some commands for generating ATAG structures in memory.
 *
 *	@author James Walmsley
 *
 *	This command forms one of the core functions of the BootThunder bootloader.
 *
 *	Note this set of commands are not re-entrant. You must run a sequence to generate
 *	a complete ATAG blob.
 *
 *	Valid sequences are:
 *
 *	1.. atag_start 0x01000000		# Initialise an ATAG blob at the address.
 *	{
 *		// Call the atag commands to construct an appropriate blob.
 *		atag_{mem|videotext|ramdisk|initrd2|serial|revision|videolfb|cmdline}
 *	}
 *	n..	atag_finish
 *
 *	Now a complete atag blob was generated, and a new blob can be started with atag_start
 *	using a different address.
 *
 *	This is useful for e.g. booting an AMP linux configuration.
 *
 **/
#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>
#include "atag.h"

static struct atag *params; /* used to point at the current tag */

static void
setup_core_tag(void * address,long pagesize)
{
    params = (struct atag *) address;         /* Initialise parameters to start at given address */

    params->hdr.tag = ATAG_CORE;            /* start with the core tag */
    params->hdr.size = tag_size(atag_core); /* size the tag */

    params->u.core.flags = 1;               /* ensure read-only */
    params->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
    params->u.core.rootdev = 0;             /* zero root device (typicaly overidden from commandline )*/

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_ramdisk_tag(BT_u32 size)
{
    params->hdr.tag = ATAG_RAMDISK;         /* Ramdisk tag */
    params->hdr.size = tag_size(atag_ramdisk);  /* size tag */

    params->u.ramdisk.flags = 0;            /* Load the ramdisk */
    params->u.ramdisk.size = size;          /* Decompressed ramdisk size */
    params->u.ramdisk.start = 0;            /* Unused */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_initrd2_tag(BT_u32 start, BT_u32 size)
{
    params->hdr.tag = ATAG_INITRD2;         /* Initrd2 tag */
    params->hdr.size = tag_size(atag_initrd2);  /* size tag */

    params->u.initrd2.start = start;        /* physical start */
    params->u.initrd2.size = size;          /* compressed ramdisk size */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_mem_tag(BT_u32 start, BT_u32 len)
{
    params->hdr.tag = ATAG_MEM;             /* Memory tag */
    params->hdr.size = tag_size(atag_mem);  /* size tag */

    params->u.mem.start = start;            /* Start of memory area (physical address) */
    params->u.mem.size = len;               /* Length of area */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_cmdline_tag(const char * line)
{
    int linelen = strlen(line);

    if(!linelen)
        return;                             /* do not insert a tag for an empty commandline */

    params->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
    params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

    strcpy(params->u.cmdline.cmdline,line); /* place commandline into tag */

    params = tag_next(params);              /* move pointer to next tag */
}

static void
setup_end_tag(void)
{
    params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    params->hdr.size = 0;                   /* zero length */
}


static int bt_atag_core(int argc, char **argv) {

	if(argc != 2) {
		bt_printf("Usage: %s [0x{start-address}]\n", argv[0]);
		return -1;
	}

	BT_u32 addr = strtol(argv[1], NULL, 16);

	setup_core_tag((void *) addr, 4096);	// Replace with BT_PAGE_SIZE when MMU config is available.

	return 0;
}

BT_SHELL_COMMAND_DEF oCoreCommand = {
	.szpName 	= "atag_start",
	.eType 	 	= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_core,
};

#ifdef BT_CONFIG_SHELL_CMD_ATAG_RAMDISK
static int bt_atag_ramdisk(int argc, char **argv) {
	if(argc != 2) {
		bt_printf("Usage: %s [0x{size(bytes)}]\n", argv[0]);
		return -1;
	}

	BT_u32 size = strtol(argv[1], NULL, 16);

	setup_ramdisk_tag(size);

	return 0;
}

BT_SHELL_COMMAND_DEF oRamDiskCommand = {
	.szpName 	= "atag_ramdisk",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_ramdisk,
};
#endif

#ifdef BT_CONFIG_SHELL_CMD_ATAG_INITRD2
static int bt_atag_initrd2(int argc, char **argv) {

	if(argc != 3) {
		bt_printf("Usage: %s [0x{start_address}] [0x{size(bytes)}]\n", argv[0]);
		return -1;
	}

	BT_u32 addr, size;
	addr = strtol(argv[1], NULL, 16);
	size = strtol(argv[1], NULL, 16);

	setup_initrd2_tag(addr, size);

	return 0;
}

BT_SHELL_COMMAND_DEF oInitrdCommand = {
	.szpName	= "atag_initrd2",
	.eType		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_initrd2,
};
#endif

#ifdef BT_CONFIG_SHELL_CMD_ATAG_MEM
static int bt_atag_mem(int argc, char **argv) {
	if(argc != 3) {
		bt_printf("Usage: %s [0x{start_address}] [0x{size(bytes)}]\n", argv[0]);
		return -1;
	}

	BT_u32 addr, size;
	addr = strtol(argv[1], NULL, 16);
	size = strtol(argv[1], NULL, 16);

	setup_mem_tag(addr, size);

	return 0;
}

BT_SHELL_COMMAND_DEF oMemCommand = {
	.szpName 	= "atag_mem",
	.eType		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_mem,
};
#endif

#ifdef BT_CONFIG_SHELL_CMD_ATAG_CMDLINE
static int bt_atag_cmdline(int argc, char **argv) {
	if(argc != 2) {
		bt_printf("Usage: %s \"[kernel command line]\"\n", argv[0]);
		return -1;
	}

	setup_cmdline_tag(argv[1]);

	return 0;
}

BT_SHELL_COMMAND_DEF oCmdlineCommand = {
	.szpName	= "atag_cmdline",
	.eType		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_cmdline,
};
#endif

static int bt_atag_finish(int argc, char **argv) {
	setup_end_tag();
	return 0;
}

BT_SHELL_COMMAND_DEF oFinishCommand = {
	.szpName 	= "atag_finish",
	.eType 		= BT_SHELL_NORMAL_COMMAND,
	.pfnCommand	= bt_atag_finish,
};
