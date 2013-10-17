/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2000, 2001  Free Software Foundation, Inc.
 *
 *  (C) Copyright 2003 Sysgo Real-Time Solutions, AG <www.elinos.com>
 *  Pavel Bartusek <pba@sysgo.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* An implementation for the Ext2FS filesystem ported from GRUB.
 * Some parts of this code (mainly the structures and defines) are
 * from the original ext2 fs code, as found in the linux kernel.
 */

#include <bitthunder.h>

#define SECTOR_SIZE		0x200
#define SECTOR_BITS		9

/* Error codes */
typedef enum
{
  ERR_NONE = 0,
  ERR_BAD_FILENAME,
  ERR_BAD_FILETYPE,
  ERR_BAD_GZIP_DATA,
  ERR_BAD_GZIP_HEADER,
  ERR_BAD_PART_TABLE,
  ERR_BAD_VERSION,
  ERR_BELOW_1MB,
  ERR_BOOT_COMMAND,
  ERR_BOOT_FAILURE,
  ERR_BOOT_FEATURES,
  ERR_DEV_FORMAT,
  ERR_DEV_VALUES,
  ERR_EXEC_FORMAT,
  ERR_FILELENGTH,
  ERR_FILE_NOT_FOUND,
  ERR_FSYS_CORRUPT,
  ERR_FSYS_MOUNT,
  ERR_GEOM,
  ERR_NEED_LX_KERNEL,
  ERR_NEED_MB_KERNEL,
  ERR_NO_DISK,
  ERR_NO_PART,
  ERR_NUMBER_PARSING,
  ERR_OUTSIDE_PART,
  ERR_READ,
  ERR_SYMLINK_LOOP,
  ERR_UNRECOGNIZED,
  ERR_WONT_FIT,
  ERR_WRITE,
  ERR_BAD_ARGUMENT,
  ERR_UNALIGNED,
  ERR_PRIVILEGED,
  ERR_DEV_NEED_INIT,
  ERR_NO_DISK_SPACE,
  ERR_NUMBER_OVERFLOW,

  MAX_ERR_NUM
} ext2fs_error_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

/* Filetype used in directory entry.  */
#define	EXT2_FILETYPE_UNKNOWN	0
#define	EXT2_FILETYPE_REG	1
#define	EXT2_FILETYPE_DIRECTORY	2
#define	EXT2_FILETYPE_SYMLINK	7

/* Amount of indirect blocks in an inode.  */
#define INDIRECT_BLOCKS		12

/* The ext2 inode.  */
struct ext2_inode {
	uint16_t mode;
	uint16_t uid;
	uint32_t size;
	uint32_t atime;
	uint32_t ctime;
	uint32_t mtime;
	uint32_t dtime;
	uint16_t gid;
	uint16_t nlinks;
	uint32_t blockcnt;	/* Blocks of 512 bytes!! */
	uint32_t flags;
	uint32_t osd1;
	union {
		struct datablocks {
			uint32_t dir_blocks[INDIRECT_BLOCKS];
			uint32_t indir_block;
			uint32_t double_indir_block;
			uint32_t tripple_indir_block;
		} blocks;
		char symlink[60];
	} b;
	uint32_t version;
	uint32_t acl;
	uint32_t dir_acl;
	uint32_t fragment_addr;
	uint32_t osd2[3];
};

typedef int (*EXT2_READ_BLOCKS)	(unsigned char *pBuffer, unsigned int SectorAddress, unsigned int Count, void *pParam);

extern void ext2fs_set_readblocks(EXT2_READ_BLOCKS fnpReadBlocks, void *pParam);
extern int ext2fs_mount (unsigned part_length);
extern int ext2fs_get_inode(char *name, struct ext2_inode *inode);
extern int ext2fs_open_dir (char *filename);
extern int ext2fs_read_dir (char *fname, int maxlen, unsigned long *fsize, int *ftype);
extern int ext2fs_close_dir (void);
extern int ext2fs_open (char *filename);
extern int ext2fs_read (char *buf, unsigned len);
extern int ext2fs_close (void);

