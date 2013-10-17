/*
 * (C) Copyright 2004
 *  esd gmbh <www.esd-electronics.com>
 *  Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 *  based on code of fs/reiserfs/dev.c by
 *
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
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


#include "ext2fs.h"
#include <string.h>

#define min(x, y) ({                            \
        typeof(x) _min1 = (x);                  \
        typeof(y) _min2 = (y);                  \
        (void) (&_min1 == &_min2);              \
        _min1 < _min2 ? _min1 : _min2; }) 

#define DEBUG 0

typedef int (*EXT2_READ_BLOCKS)	(unsigned char *pBuffer, unsigned int SectorAddress, unsigned int Count, void *pParam);

static EXT2_READ_BLOCKS ext2fs_read_blocks = NULL;
static void *ext2fs_read_blocks_param = NULL;

void ext2fs_set_readblocks(EXT2_READ_BLOCKS fnpReadBlocks, void *pParam)
{
	ext2fs_read_blocks = fnpReadBlocks;
	ext2fs_read_blocks_param = pParam;
}

int ext2fs_devread (int sector, int byte_offset, int byte_len, char *buf) {
	char sec_buf[SECTOR_SIZE];
	unsigned block_len;

/*
 *  Get the read to the beginning of a partition.
 */
	sector += byte_offset >> SECTOR_BITS;
	byte_offset &= SECTOR_SIZE - 1;

#if DEBUG
	bt_printf(" <%d, %d, %d>\n", sector, byte_offset, byte_len);
#endif

	if (!ext2fs_read_blocks) {
		bt_printf ("** Invalid Read Blocks Function Pointer (NULL)\n");
		return (0);
	}

	if (byte_offset != 0) {
		/* read first part which isn't aligned with start of sector */
		if (ext2fs_read_blocks ((unsigned char *) sec_buf, sector, 1, ext2fs_read_blocks_param) != 1) {
			bt_printf (" ** ext2fs_devread() read error **\n");
			return (0);
		}
		memcpy (buf, sec_buf + byte_offset, min (SECTOR_SIZE - byte_offset, byte_len));
		buf += min (SECTOR_SIZE - byte_offset, byte_len);
		byte_len -= min (SECTOR_SIZE - byte_offset, byte_len);
		sector++;
	}

	if (byte_len == 0)
		return 1;

	/*  read sector aligned part */
	block_len = byte_len & ~(SECTOR_SIZE - 1);

	if (block_len == 0) {
		unsigned char p[SECTOR_SIZE];

		block_len = SECTOR_SIZE;
		ext2fs_read_blocks ((unsigned char *)p, sector, 1, ext2fs_read_blocks_param);
		memcpy(buf, p, byte_len);
		return 1;
	}

	if (ext2fs_read_blocks ((unsigned char *)buf, sector, block_len / SECTOR_SIZE, ext2fs_read_blocks_param) != block_len / SECTOR_SIZE) {
		bt_printf (" ** ext2fs_devread() read error - block\n");
		return (0);
	}
	block_len = byte_len & ~(SECTOR_SIZE - 1);
	buf += block_len;
	byte_len -= block_len;
	sector += block_len / SECTOR_SIZE;

	if (byte_len != 0) {
		/* read rest of data which are not in whole sector */
		if (ext2fs_read_blocks ((unsigned char *)sec_buf, sector, 1, ext2fs_read_blocks_param) != 1) {
			bt_printf (" ** ext2fs_devread() read error - last part\n");
			return (0);
		}
		memcpy (buf, sec_buf, byte_len);
	}
	return (1);
}
