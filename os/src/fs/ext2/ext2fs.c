/*
 * (C) Copyright 2004
 *  esd gmbh <www.esd-electronics.com>
 *  Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 *  based on code from grub2 fs/ext2.c and fs/fshelp.c by
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003, 2004  Free Software Foundation, Inc.
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

#define DEBUG 0

#define EXT2FS_DEVREAD_CACHE_SIZE 5

struct ext2fs_devread_cache_data {
	int sector, byte_offset, byte_len, rc;
	char *buf;
};

extern int ext2fs_devread (int sector, int byte_offset, int byte_len, char *buf);

static int ext2fs_devread_cached (int sector, int byte_offset, int byte_len, char *buf, struct ext2fs_devread_cache_data *cd)
{
	int i, rc=1;

	for (i=0; i<EXT2FS_DEVREAD_CACHE_SIZE; i++)
		if (cd[i].buf && cd[i].sector == sector && cd[i].byte_offset == byte_offset && cd[i].byte_len == byte_len) {
			struct ext2fs_devread_cache_data tmp = cd[i];
			memcpy(buf, tmp.buf, byte_len);
			while (i > 0) {
				cd[i] = cd[i-1];
				i--;
			}
			cd[0] = tmp;
#if DEBUG
			BT_kPrint("- CACHE HIT -\n");
#endif
			return rc;
		}

	rc = ext2fs_devread(sector, byte_offset, byte_len, buf);

	if(cd[EXT2FS_DEVREAD_CACHE_SIZE-1].buf) BT_kFree(cd[EXT2FS_DEVREAD_CACHE_SIZE-1].buf);
	for (i=EXT2FS_DEVREAD_CACHE_SIZE-1; i>0; i--)
		cd[i] = cd[i-1];
	cd[0].sector = sector;
	cd[0].byte_offset = byte_offset;
	cd[0].byte_len = byte_len;
	cd[0].rc = rc;
	cd[0].buf = BT_kMalloc(byte_len);
	memcpy(cd[0].buf, buf, byte_len);
#if DEBUG
	BT_kPrint("- CACHE MISS -\n");
#endif
	return rc;
}

static void ext2fs_devread_freecache(struct ext2fs_devread_cache_data *cd)
{
	int i;
	for (i=0; i<EXT2FS_DEVREAD_CACHE_SIZE; i++) {
		if(cd[i].buf) BT_kFree(cd[i].buf);
		cd[i].buf = 0;
	}
}


/* The good old revision and the default inode size.  */
#define EXT2_GOOD_OLD_REVISION		0
#define EXT2_GOOD_OLD_INODE_SIZE	128

/* The revision level.  */
#define EXT2_REVISION(data)	bt_le32_to_cpu (data->sblock.revision_level)

#define EXT2_INODE_SIZE(data)	\
        (EXT2_REVISION (data) == EXT2_GOOD_OLD_REVISION \
         ? EXT2_GOOD_OLD_INODE_SIZE \
         : bt_le16_to_cpu (data->sblock.inode_size))


/* Magic value used to identify an ext2 filesystem.  */
#define	EXT2_MAGIC		0xEF53
/* Maximum lenght of a pathname.  */
#define EXT2_PATH_MAX		4096
/* Maximum nesting of symlinks, used to prevent a loop.  */
#define	EXT2_MAX_SYMLINKCNT	8

/* Filetype information as used in inodes.  */
#define FILETYPE_INO_MASK	0170000
#define FILETYPE_INO_REG	0100000
#define FILETYPE_INO_DIRECTORY	0040000
#define FILETYPE_INO_SYMLINK	0120000

/* Bits used as offset in sector */
#define DISK_SECTOR_BITS        9

/* Log2 size of ext2 block in 512 blocks.  */
#define LOG2_EXT2_BLOCK_SIZE(data) (bt_le32_to_cpu (data->sblock.log2_block_size) + 1)

/* Log2 size of ext2 block in bytes.  */
#define LOG2_BLOCK_SIZE(data)	   (bt_le32_to_cpu (data->sblock.log2_block_size) + 10)

/* The size of an ext2 block in bytes.  */
#define EXT2_BLOCK_SIZE(data)	   (1 << LOG2_BLOCK_SIZE(data))

/* The ext2 superblock.  */
struct ext2_sblock {
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t reserved_blocks;
	uint32_t free_blocks;
	uint32_t free_inodes;
	uint32_t first_data_block;
	uint32_t log2_block_size;
	uint32_t log2_fragment_size;
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	uint32_t mtime;
	uint32_t utime;
	uint16_t mnt_count;
	uint16_t max_mnt_count;
	uint16_t magic;
	uint16_t fs_state;
	uint16_t error_handling;
	uint16_t minor_revision_level;
	uint32_t lastcheck;
	uint32_t checkinterval;
	uint32_t creator_os;
	uint32_t revision_level;
	uint16_t uid_reserved;
	uint16_t gid_reserved;
	uint32_t first_inode;
	uint16_t inode_size;
	uint16_t block_group_number;
	uint32_t feature_compatibility;
	uint32_t feature_incompat;
	uint32_t feature_ro_compat;
	uint32_t unique_id[4];
	char volume_name[16];
	char last_mounted_on[64];
	uint32_t compression_info;
};

/* The ext2 blockgroup.  */
struct ext2_block_group {
	uint32_t block_id;
	uint32_t inode_id;
	uint32_t inode_table_id;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t used_dirs_count;
	uint32_t reserved[3];
};

/* The header of an ext2 directory entry.  */
struct ext2_dirent {
	uint32_t inode;
	uint16_t direntlen;
	uint8_t namelen;
	uint8_t filetype;
};

struct ext2fs_node {
	struct ext2_data *data;
	struct ext2_inode inode;
	int ino;
	int inode_read;
};

/* Information about a "mounted" ext2 filesystem.  */
struct ext2_data {
	struct ext2_sblock sblock;
	struct ext2_inode *inode;
	struct ext2fs_node diropen;
};

typedef struct ext2fs_node *ext2fs_node_t;
static struct ext2_data *ext2fs_root = NULL;
static ext2fs_node_t ext2fs_dir = NULL;
static unsigned int ext2fs_dir_fpos = 0;
static ext2fs_node_t ext2fs_file = NULL;
static unsigned int ext2fs_fpos = 0;
static struct ext2fs_devread_cache_data ext2fs_cd[EXT2FS_DEVREAD_CACHE_SIZE] = { };
static int symlinknest = 0;
static uint32_t *indir1_block = NULL;
static int indir1_size = 0;
static int indir1_blkno = -1;
static uint32_t *indir2_block = NULL;
static int indir2_size = 0;
static int indir2_blkno = -1;


static int ext2fs_blockgroup
	(struct ext2_data *data, int group, struct ext2_block_group *blkgrp) {
	int rc = (ext2fs_devread
		(((bt_le32_to_cpu (data->sblock.first_data_block) +
		   1) << LOG2_EXT2_BLOCK_SIZE (data)),
		 group * sizeof (struct ext2_block_group),
		 sizeof (struct ext2_block_group), (char *) blkgrp));
#if DEBUG
	BT_kPrint ("ext2fs read blockgroup (rc = %d)\n", rc);
	BT_kPrint("  block_id = %d\n", ((struct ext2_block_group *)blkgrp)->block_id);
	BT_kPrint("  inode_id = %d\n", ((struct ext2_block_group *)blkgrp)->inode_id);
	BT_kPrint("  inode_table_id = %d\n", ((struct ext2_block_group *)blkgrp)->inode_table_id);
	BT_kPrint("  free_blocks = %d\n", ((struct ext2_block_group *)blkgrp)->free_blocks);
	BT_kPrint("  free_inodes = %d\n", ((struct ext2_block_group *)blkgrp)->free_inodes);
	BT_kPrint("  used_dirs_count = %d\n", ((struct ext2_block_group *)blkgrp)->used_dirs_count);
#endif

	return rc;
}


static int ext2fs_read_inode
	(struct ext2_data *data, int ino, struct ext2_inode *inode) {
	struct ext2_block_group blkgrp;
	struct ext2_sblock *sblock = &data->sblock;
	int inodes_per_block;
	int status;

	unsigned int blkno;
	unsigned int blkoff;

	/* It is easier to calculate if the first inode is 0.  */
	ino--;
#if DEBUG
	BT_kPrint ("ext2fs read inode %d\n", ino);
#endif
	status = ext2fs_blockgroup (data,
				    ino /
				    bt_le32_to_cpu (sblock->inodes_per_group),
				    &blkgrp);
	if (status == 0) {
		return (0);
	}
	inodes_per_block = EXT2_BLOCK_SIZE (data) / EXT2_INODE_SIZE (data);
	blkno = (ino % bt_le32_to_cpu (sblock->inodes_per_group)) /
		inodes_per_block;
	blkoff = (ino % bt_le32_to_cpu (sblock->inodes_per_group)) %
		inodes_per_block;
#if DEBUG
	BT_kPrint ("ext2fs read inode blkno %d blkoff %d\n", blkno, blkoff);
#endif
	/* Read the inode.  */
	status = ext2fs_devread (((bt_le32_to_cpu (blkgrp.inode_table_id) +
				   blkno) << LOG2_EXT2_BLOCK_SIZE (data)),
		                 EXT2_INODE_SIZE (data) * blkoff,
				 sizeof (struct ext2_inode), (char *) inode);
	if (status == 0) {
		return (0);
	}
	
#if DEBUG
	BT_kPrint ("  mode = %d\n", inode->mode);
	BT_kPrint ("  uid = %d\n", inode->uid);
	BT_kPrint ("  size = %d\n", inode->size);
	BT_kPrint ("  atime = %d\n", inode->atime);
	BT_kPrint ("  ctime = %d\n", inode->ctime);
	BT_kPrint ("  mtime = %d\n", inode->mtime);
	BT_kPrint ("  dtime = %d\n", inode->dtime);
	BT_kPrint ("  gid = %d\n", inode->gid);
	BT_kPrint ("  nlinks = %d\n", inode->nlinks);
	BT_kPrint ("  blockcnt = %d\n", inode->blockcnt);
	BT_kPrint ("  flags = %d\n", inode->flags);
	BT_kPrint ("  osd1 = %d\n", inode->osd1);
	int i;
	for(i=0;i<INDIRECT_BLOCKS;i++) BT_kPrint ("  dir_blocks[%d] = %d\n", i, inode->b.blocks.dir_blocks[i]);
	BT_kPrint ("  indir_blocks = %d\n", inode->b.blocks.indir_block);
	BT_kPrint ("  double_indir_blocks = %d\n", inode->b.blocks.double_indir_block);
	BT_kPrint ("  tripple_indir_blocks = %d\n", inode->b.blocks.tripple_indir_block);
	BT_kPrint ("  acl = %d\n", inode->acl);
	BT_kPrint ("  dir_acl = %d\n", inode->dir_acl);
	BT_kPrint ("  fragment_addr = %d\n", inode->fragment_addr);
	BT_kPrint ("  osd2 = %d %d %d\n", inode->osd2[0], inode->osd2[1], inode->osd2[2]);
#endif

	return (1);
}


void ext2fs_free_node (ext2fs_node_t node, ext2fs_node_t currroot) {
	if ((node != &ext2fs_root->diropen) && (node != currroot)) {
		BT_kFree(node);
	}
}


static int ext2fs_read_block (ext2fs_node_t node, int fileblock, struct ext2fs_devread_cache_data *cd) {
	struct ext2_data *data = node->data;
	struct ext2_inode *inode = &node->inode;
	int blknr;
	int blksz = EXT2_BLOCK_SIZE (data);
	int log2_blksz = LOG2_EXT2_BLOCK_SIZE (data);
	int status;

	//bt_printf("ext2fs_read_block %d ...\n", fileblock);

	/* Direct blocks.  */
	if (fileblock < INDIRECT_BLOCKS) {
		blknr = bt_le32_to_cpu (inode->b.blocks.dir_blocks[fileblock]);
	}
	/* Indirect.  */
	else if (fileblock < (INDIRECT_BLOCKS + (blksz / 4))) {
		if (indir1_block == NULL) {
			indir1_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir1_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 1) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir1_size = blksz;
			indir1_blkno = -1;
		}
		if (blksz != indir1_size) {
			BT_kFree(indir1_block);
			indir1_block = NULL;
			indir1_size = 0;
			indir1_blkno = -1;
			indir1_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir1_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 1) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir1_size = blksz;
		}
		if ((bt_le32_to_cpu (inode->b.blocks.indir_block) <<
		     log2_blksz) != indir1_blkno) {
			status = ext2fs_devread_cached (bt_le32_to_cpu(inode->b.blocks.indir_block) << log2_blksz,
						 0, blksz,
						 (char *) indir1_block, cd);
			if (status == 0) {
				BT_kPrint ("** ext2fs read block (indir 1) failed. **\n");
				return (0);
			}
			indir1_blkno =
				bt_le32_to_cpu (inode->b.blocks.
					       indir_block) << log2_blksz;
		}
		blknr = bt_le32_to_cpu (indir1_block
				       [fileblock - INDIRECT_BLOCKS]);
	}
	/* Double indirect.  */
	else if (fileblock <
		 (INDIRECT_BLOCKS + (blksz / 4 * (blksz / 4 + 1)))) {
		unsigned int perblock = blksz / 4;
		unsigned int rblock = fileblock - (INDIRECT_BLOCKS
						   + blksz / 4);

		if (indir1_block == NULL) {
			indir1_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir1_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 2 1) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir1_size = blksz;
			indir1_blkno = -1;
		}
		if (blksz != indir1_size) {
			BT_kFree(indir1_block);
			indir1_block = NULL;
			indir1_size = 0;
			indir1_blkno = -1;
			indir1_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir1_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 2 1) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir1_size = blksz;
		}
		if ((bt_le32_to_cpu (inode->b.blocks.double_indir_block) <<
		     log2_blksz) != indir1_blkno) {
			status = ext2fs_devread_cached (bt_le32_to_cpu(inode->b.blocks.double_indir_block) << log2_blksz,
						0, blksz,
						(char *) indir1_block, cd);
			if (status == 0) {
				BT_kPrint ("** ext2fs read block (indir 2 1) failed. **\n");
				return (-1);
			}
			indir1_blkno =
				bt_le32_to_cpu (inode->b.blocks.double_indir_block) << log2_blksz;
		}

		if (indir2_block == NULL) {
			indir2_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir2_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 2 2) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir2_size = blksz;
			indir2_blkno = -1;
		}
		if (blksz != indir2_size) {
			BT_kFree(indir2_block);
			indir2_block = NULL;
			indir2_size = 0;
			indir2_blkno = -1;
			indir2_block = (uint32_t *) BT_kMalloc (blksz);
			if (indir2_block == NULL) {
				BT_kPrint ("** ext2fs read block (indir 2 2) BT_kMalloc failed. **\n");
				return (-1);
			}
			indir2_size = blksz;
		}
		if ((bt_le32_to_cpu (indir1_block[rblock / perblock]) <<
		     log2_blksz) != indir1_blkno) {
			status = ext2fs_devread_cached (bt_le32_to_cpu(indir1_block[rblock / perblock]) << log2_blksz,
						 0, blksz,
						 (char *) indir2_block, cd);
			if (status == 0) {
				BT_kPrint ("** ext2fs read block (indir 2 2) failed. **\n");
				return (-1);
			}
			indir2_blkno =
				bt_le32_to_cpu (indir1_block[rblock / perblock]) << log2_blksz;
		}
		blknr = bt_le32_to_cpu (indir2_block[rblock % perblock]);
	}
	/* Tripple indirect.  */
	else {
		BT_kPrint ("** ext2fs doesn't support tripple indirect blocks. **\n");
		return (-1);
	}
#if DEBUG
	BT_kPrint ("ext2fs_read_block %08x\n", blknr);
#endif
	return (blknr);
}


int ext2fs_read_file
	(ext2fs_node_t node, int pos, unsigned int len, char *buf) {
	int i;
	int blockcnt;
	int log2blocksize = LOG2_EXT2_BLOCK_SIZE (node->data);
	int blocksize = 1 << (log2blocksize + DISK_SECTOR_BITS);
	unsigned int filesize = bt_le32_to_cpu(node->inode.size);

	/* Adjust len so it we can't read past the end of the file.  */
	if (pos + len > filesize) {
		len = filesize - pos;
	}

	if(len > 0) {
		blockcnt = ((len + pos) + blocksize - 1) / blocksize;

		for (i = pos / blocksize; i < blockcnt; i++) {
			int blknr;
			int blockoff = pos % blocksize;
			int blockend = blocksize;

			int skipfirst = 0;

			blknr = ext2fs_read_block (node, i, ext2fs_cd);
			if (blknr < 0) {
				ext2fs_devread_freecache(ext2fs_cd);
				return (-1);
			}
			blknr = blknr << log2blocksize;

			/* Last block.  */
			if (i == blockcnt - 1) {
				blockend = (len + pos) % blocksize;

				/* The last portion is exactly blocksize.  */
				if (!blockend) {
					blockend = blocksize;
				}
			}

			/* First block.  */
			if (i == pos / blocksize) {
				skipfirst = blockoff;
				blockend -= skipfirst;
			}

			/* If the block number is 0 this block is not stored on disk but
			   is zero filled instead.  */
			if (blknr) {
				int status;

				status = ext2fs_devread (blknr, skipfirst, blockend, buf);
				if (status == 0) {
					ext2fs_devread_freecache(ext2fs_cd);
					return (-1);
				}
			} else {
				memset (buf, 0, blocksize - skipfirst);
			}
			buf += blocksize - skipfirst;
		}

		ext2fs_devread_freecache(ext2fs_cd);
	}

	return (len);
}


static int ext2fs_iterate_dir (ext2fs_node_t dir, char *name, ext2fs_node_t * fnode, int *ftype)
{
	unsigned int fpos = 0;
	int status;
	struct ext2fs_node *diro = (struct ext2fs_node *) dir;

	if ((name == NULL) || (fnode == NULL) || (ftype == NULL)) {
		return 0;
	}
	
#if DEBUG
	if (name != NULL)
		BT_kPrint ("Iterate dir %s\n", name);
#endif /* of DEBUG */
	if (!diro->inode_read) {
		status = ext2fs_read_inode (diro->data, diro->ino,
					    &diro->inode);
		if (status == 0) {
			BT_kPrint("** Failed to read inode **\n");
			return (0);
		}
	}
	/* Search the file.  */
	while (fpos < bt_le32_to_cpu (diro->inode.size)) {
		struct ext2_dirent dirent;

		status = ext2fs_read_file (diro, fpos,
					   sizeof (struct ext2_dirent),
					   (char *) &dirent);
		if (status < 1) {
			BT_kPrint("** Failed to read file **\n");
			return (0);
		}
		if (dirent.namelen != 0) {
			char filename[dirent.namelen + 1];
			ext2fs_node_t fdiro;
			int type = EXT2_FILETYPE_UNKNOWN;

			status = ext2fs_read_file (diro,
						   fpos + sizeof (struct ext2_dirent),
						   dirent.namelen, filename);
			if (status < 1) {
				BT_kPrint("** Failed to read file 2 **\n");
				return (0);
			}
			fdiro = BT_kMalloc (sizeof (struct ext2fs_node));
			if (!fdiro) {
				BT_kPrint("** Failed to malloc **\n");
				return (0);
			}

			fdiro->data = diro->data;
			fdiro->ino = bt_le32_to_cpu (dirent.inode);

			filename[dirent.namelen] = '\0';

			if (dirent.filetype != EXT2_FILETYPE_UNKNOWN) {
				fdiro->inode_read = 0;

				if (dirent.filetype == EXT2_FILETYPE_DIRECTORY) {
					type = EXT2_FILETYPE_DIRECTORY;
				} else if (dirent.filetype ==
					   EXT2_FILETYPE_SYMLINK) {
					type = EXT2_FILETYPE_SYMLINK;
				} else if (dirent.filetype == EXT2_FILETYPE_REG) {
					type = EXT2_FILETYPE_REG;
				}
			} else {
				/* The filetype can not be read from the dirent, get it from inode */

				status = ext2fs_read_inode (diro->data,
							    bt_le32_to_cpu(dirent.inode),
							    &fdiro->inode);
				if (status == 0) {
					BT_kFree(fdiro);
					BT_kPrint("** Failed to read inode 2 **\n");
					return (0);
				}
				fdiro->inode_read = 1;

				if ((bt_le16_to_cpu (fdiro->inode.mode) &
				     FILETYPE_INO_MASK) ==
				    FILETYPE_INO_DIRECTORY) {
					type = EXT2_FILETYPE_DIRECTORY;
				} else if ((bt_le16_to_cpu (fdiro->inode.mode)
					    & FILETYPE_INO_MASK) ==
					   FILETYPE_INO_SYMLINK) {
					type = EXT2_FILETYPE_SYMLINK;
				} else if ((bt_le16_to_cpu (fdiro->inode.mode)
					    & FILETYPE_INO_MASK) ==
					   FILETYPE_INO_REG) {
					type = EXT2_FILETYPE_REG;
				}
			}
#if DEBUG
			BT_kPrint ("iterate >%s<\n", filename);
#endif /* of DEBUG */
			if (strcmp (filename, name) == 0) {
				*ftype = type;
				*fnode = fdiro;
				return (1);
			}

			BT_kFree(fdiro);
		}
		fpos += bt_le16_to_cpu (dirent.direntlen);
	}
	return (0);
}


static char *ext2fs_read_symlink (ext2fs_node_t node) {
	char *symlink;
	struct ext2fs_node *diro = node;
	int status;

	if (!diro->inode_read) {
		status = ext2fs_read_inode (diro->data, diro->ino,
					    &diro->inode);
		if (status == 0) {
			return (0);
		}
	}
	symlink = BT_kMalloc (bt_le32_to_cpu (diro->inode.size) + 1);
	if (!symlink) {
		return (0);
	}
	/* If the filesize of the symlink is bigger than
	   60 the symlink is stored in a separate block,
	   otherwise it is stored in the inode.  */
	if (bt_le32_to_cpu (diro->inode.size) <= 60) {
		strncpy (symlink, diro->inode.b.symlink,
			 bt_le32_to_cpu (diro->inode.size));
	} else {
		status = ext2fs_read_file (diro, 0,
					   bt_le32_to_cpu (diro->inode.size),
					   symlink);
		if (status == 0) {
			BT_kFree(symlink);
			return (0);
		}
	}
	symlink[bt_le32_to_cpu (diro->inode.size)] = '\0';
	return (symlink);
}


int ext2fs_find_file1
	(const char *currpath,
	 ext2fs_node_t currroot, ext2fs_node_t * currfound, int *foundtype) {
	char fpath[strlen (currpath) + 1];
	char *name = fpath;
	char *next;
	int status;
	int type = EXT2_FILETYPE_DIRECTORY;
	ext2fs_node_t currnode = currroot;
	ext2fs_node_t oldnode = currroot;

	strncpy (fpath, currpath, strlen (currpath) + 1);

	/* Remove all leading slashes.  */
	while (*name == '/') {
		name++;
	}
	if (!*name) {
		*currfound = currnode;
		return (1);
	}

	for (;;) {
		int found;

		/* Extract the actual part from the pathname.  */
		next = strchr (name, '/');
		if (next) {
			/* Remove all leading slashes.  */
			while (*next == '/') {
				*(next++) = '\0';
			}
		}

		/* At this point it is expected that the current node is a directory, check if this is true.  */
		if (type != EXT2_FILETYPE_DIRECTORY) {
			ext2fs_free_node (currnode, currroot);
			return (0);
		}

		oldnode = currnode;

		/* Iterate over the directory.  */
		found = ext2fs_iterate_dir (currnode, name, &currnode, &type);
		if (found == 0) {
			ext2fs_free_node (currnode, currroot);
			return (0);
		}
		if (found == -1) {
			break;
		}

		/* Read in the symlink and follow it.  */
		if (type == EXT2_FILETYPE_SYMLINK) {
			char *symlink;

			/* Test if the symlink does not loop.  */
			if (++symlinknest == 8) {
				ext2fs_free_node (currnode, currroot);
				ext2fs_free_node (oldnode, currroot);
				return (0);
			}

			symlink = ext2fs_read_symlink (currnode);
			ext2fs_free_node (currnode, currroot);

			if (!symlink) {
				ext2fs_free_node (oldnode, currroot);
				return (0);
			}
#if DEBUG
			BT_kPrint ("Got symlink >%s<\n", symlink);
#endif /* of DEBUG */
			/* The symlink is an absolute path, go back to the root inode.  */
			if (symlink[0] == '/') {
				ext2fs_free_node (oldnode, currroot);
				oldnode = &ext2fs_root->diropen;
			}

			/* Lookup the node the symlink points to.  */
			status = ext2fs_find_file1 (symlink, oldnode,
						    &currnode, &type);

			BT_kFree(symlink);

			if (status == 0) {
				ext2fs_free_node (oldnode, currroot);
				return (0);
			}
		}

		ext2fs_free_node (oldnode, currroot);

		/* Found the node!  */
		if (!next || *next == '\0') {
			*currfound = currnode;
			*foundtype = type;
			return (1);
		}
		name = next;
	}
	return (-1);
}


int ext2fs_find_file
	(const char *path,
	 ext2fs_node_t rootnode, ext2fs_node_t * foundnode, int expecttype) {
	int status;
	int foundtype = EXT2_FILETYPE_DIRECTORY;


	symlinknest = 0;
	if (!path) {
		return (0);
	}

	status = ext2fs_find_file1 (path, rootnode, foundnode, &foundtype);
	if (status == 0) {
		return (0);
	}
	/* Check if the node that was found was of the expected type.  */
	if ((expecttype == EXT2_FILETYPE_REG) && (foundtype != expecttype)) {
		return (0);
	} else if ((expecttype == EXT2_FILETYPE_DIRECTORY)
		   && (foundtype != expecttype)) {
		return (0);
	}
	return (1);
}

int ext2fs_get_inode(char *name, struct ext2_inode *inode) {
	ext2fs_node_t fnode;
	int status;

	if (ext2fs_root == NULL) {
		return -1;
	}

	status = ext2fs_find_file (name, &ext2fs_root->diropen, &fnode,
				   EXT2_FILETYPE_REG);
	if (status != 1) {
		BT_kPrint ("** Can not find file. **\n");
		return -1;
	}

	if (!fnode->inode_read) {
		status = ext2fs_read_inode (fnode->data, fnode->ino, &fnode->inode);
		if (status == 0) {
			return -1;
		}
	}

	if(inode) *inode = fnode->inode;

	ext2fs_free_node (fnode, &ext2fs_root->diropen);

	return 0;
}

int ext2fs_open_dir (char *dirname) {
	ext2fs_node_t dirnode;
	int status;

	if (ext2fs_root == NULL) {
		return (-1);
	}

	status = ext2fs_find_file (dirname, &ext2fs_root->diropen, &dirnode,
				   EXT2_FILETYPE_DIRECTORY);
	if (status != 1) {
		BT_kPrint ("** Can not find directory. **\n");
		return (-1);
	}

	if (!dirnode->inode_read) {
		status = ext2fs_read_inode (dirnode->data, dirnode->ino, &dirnode->inode);
		if (status == 0) {
			return (-1);
		}
	}
	ext2fs_dir = dirnode;
	ext2fs_dir_fpos = 0;
	
	return (0);
}

int ext2fs_read_dir (char *fname, int maxlen, unsigned long *fsize, int *ftype) {
	int status;

	if(fname) *fname = 0;
	if(fsize) *fsize = 0;
	if(ftype) *ftype = 0;

	if (ext2fs_dir_fpos < bt_le32_to_cpu (ext2fs_dir->inode.size)) {
		struct ext2_dirent dirent;

		status = ext2fs_read_file (ext2fs_dir, ext2fs_dir_fpos, sizeof (struct ext2_dirent), (char *) &dirent);
		if (status < 1) {
			return (-1);
		}
		if (dirent.namelen != 0) {
			char filename[dirent.namelen + 1];
			ext2fs_node_t fdiro;
			int type = EXT2_FILETYPE_UNKNOWN;

			status = ext2fs_read_file (ext2fs_dir, ext2fs_dir_fpos + sizeof (struct ext2_dirent), dirent.namelen, filename);
			if (status < 1) {
				return (-1);
			}
			fdiro = BT_kMalloc (sizeof (struct ext2fs_node));
			if (!fdiro) {
				return (-1);
			}

			fdiro->data = ext2fs_dir->data;
			fdiro->ino = bt_le32_to_cpu (dirent.inode);

			filename[dirent.namelen] = '\0';

			if (dirent.filetype != EXT2_FILETYPE_UNKNOWN) {
				fdiro->inode_read = 0;

				if (dirent.filetype == EXT2_FILETYPE_DIRECTORY) {
					type = EXT2_FILETYPE_DIRECTORY;
				} else if (dirent.filetype == EXT2_FILETYPE_SYMLINK) {
					type = EXT2_FILETYPE_SYMLINK;
				} else if (dirent.filetype == EXT2_FILETYPE_REG) {
					type = EXT2_FILETYPE_REG;
				}
			} else {
				/* The filetype can not be read from the dirent, get it from inode */

				status = ext2fs_read_inode (ext2fs_dir->data, bt_le32_to_cpu(dirent.inode), &fdiro->inode);
				if (status == 0) {
					BT_kFree(fdiro);
					return (-1);
				}
				fdiro->inode_read = 1;

				if ((bt_le16_to_cpu (fdiro->inode.mode) & FILETYPE_INO_MASK) == FILETYPE_INO_DIRECTORY) {
					type = EXT2_FILETYPE_DIRECTORY;
				} else if ((bt_le16_to_cpu (fdiro->inode.mode) & FILETYPE_INO_MASK) == FILETYPE_INO_SYMLINK) {
					type = EXT2_FILETYPE_SYMLINK;
				} else if ((bt_le16_to_cpu (fdiro->inode.mode) & FILETYPE_INO_MASK) == FILETYPE_INO_REG) {
					type = EXT2_FILETYPE_REG;
				}
			}
#if DEBUG
			BT_kPrint ("iterate >%s<\n", filename);
#endif /* of DEBUG */

			if (fdiro->inode_read == 0) {
				status = ext2fs_read_inode (ext2fs_dir->data, bt_le32_to_cpu (dirent.inode), &fdiro->inode);
				if (status == 0) {
					BT_kFree(fdiro);
					return (-1);
				}
				fdiro->inode_read = 1;
			}
			if(fname) {
				strncpy(fname, filename, maxlen);
				fname[maxlen-1]=0;
			}
			if(fsize) *fsize = bt_le32_to_cpu (fdiro->inode.size);
			if(ftype) *ftype = type;
				
			BT_kFree(fdiro);
		}
		ext2fs_dir_fpos += bt_le16_to_cpu (dirent.direntlen);
	} else {
		return (-1);
	}

	return (0);
}

int ext2fs_close_dir(void) {
	if(ext2fs_dir) {
		ext2fs_free_node (ext2fs_dir, &ext2fs_root->diropen);
		ext2fs_dir = NULL;
		ext2fs_dir_fpos = 0;
	}
	return (0);
}

int ext2fs_open (char *filename) {
	ext2fs_node_t fdiro = NULL;
	int status;
	int len;

	if (ext2fs_root == NULL) {
		return (-1);
	}
	ext2fs_file = NULL;
	status = ext2fs_find_file (filename, &ext2fs_root->diropen, &fdiro,
				   EXT2_FILETYPE_REG);
	if (status == 0) {
		goto fail;
	}
	if (!fdiro->inode_read) {
		status = ext2fs_read_inode (fdiro->data, fdiro->ino,
					    &fdiro->inode);
		if (status == 0) {
			goto fail;
		}
	}
	len = bt_le32_to_cpu (fdiro->inode.size);
	ext2fs_file = fdiro;
	ext2fs_fpos = 0;
	ext2fs_devread_freecache(ext2fs_cd);

	return (len);

fail:
	ext2fs_free_node (fdiro, &ext2fs_root->diropen);
	return (-1);
}


int ext2fs_close (void
	) {
	if ((ext2fs_file != NULL) && (ext2fs_root != NULL)) {
		ext2fs_free_node (ext2fs_file, &ext2fs_root->diropen);
		ext2fs_file = NULL;
	}
	if (indir1_block != NULL) {
		BT_kFree(indir1_block);
		indir1_block = NULL;
		indir1_size = 0;
		indir1_blkno = -1;
	}
	if (indir2_block != NULL) {
		BT_kFree(indir2_block);
		indir2_block = NULL;
		indir2_size = 0;
		indir2_blkno = -1;
	}
	ext2fs_fpos = 0;
	ext2fs_devread_freecache(ext2fs_cd);
	return (0);
}


int ext2fs_read (char *buf, unsigned len) {
	int status;

	if (ext2fs_root == NULL) {
		return (-1);
	}

	if (ext2fs_file == NULL) {
		return (-1);
	}

	status = ext2fs_read_file (ext2fs_file, ext2fs_fpos, len, buf);
	if(status > 0) ext2fs_fpos += status;

	return (status);
}


int ext2fs_mount (unsigned part_length) {
	struct ext2_data *data;
	int status;

	data = BT_kMalloc (sizeof (struct ext2_data));
	if (!data) {
		return (-1);
	}
	/* Read the superblock.  */
	status = ext2fs_devread (1 * 2, 0, sizeof (struct ext2_sblock),
				 (char *) &data->sblock);
	if (status == 0) {
		goto fail;
	}

#if DEBUG
        BT_kPrint ("ext2fs read superblock\n");
	BT_kPrint ("  total_inodes = %d\n", data->sblock.total_inodes); 
	BT_kPrint ("  total_blocks = %d\n", data->sblock.total_blocks); 
	BT_kPrint ("  free_blocks = %d\n", data->sblock.free_blocks); 
	BT_kPrint ("  free_inodes = %d\n", data->sblock.free_inodes); 
	BT_kPrint ("  first_data_block = %d\n", data->sblock.first_data_block); 
	BT_kPrint ("  log2_block_size = %d\n", data->sblock.log2_block_size); 
	BT_kPrint ("  log2_fragment_size = %d\n", data->sblock.log2_fragment_size); 
	BT_kPrint ("  blocks_per_group = %d\n", data->sblock.blocks_per_group); 
	BT_kPrint ("  fragments_per_group = %d\n", data->sblock.fragments_per_group); 
	BT_kPrint ("  inodes_per_group = %d\n", data->sblock.inodes_per_group); 
	BT_kPrint ("  mtime = %d\n", data->sblock.mtime); 
	BT_kPrint ("  utime = %d\n", data->sblock.utime); 
	BT_kPrint ("  mnt_count = %d\n", data->sblock.mnt_count); 
	BT_kPrint ("  max_mnt_count = %d\n", data->sblock.max_mnt_count); 
	BT_kPrint ("  magic = 0x%04x\n", data->sblock.magic); 
	BT_kPrint ("  fs_state = %d\n", data->sblock.fs_state); 
	BT_kPrint ("  error_handling = %d\n", data->sblock.error_handling); 
	BT_kPrint ("  minor_revision_level = %d\n", data->sblock.minor_revision_level); 
	BT_kPrint ("  lastcheck = %d\n", data->sblock.lastcheck); 
	BT_kPrint ("  checkinterval = %d\n", data->sblock.checkinterval); 
	BT_kPrint ("  creator_os = %d\n", data->sblock.creator_os); 
	BT_kPrint ("  revision_level = %d\n", data->sblock.revision_level); 
	BT_kPrint ("  uid_reserved = %d\n", data->sblock.uid_reserved); 
	BT_kPrint ("  gid_reserved = %d\n", data->sblock.gid_reserved); 
	BT_kPrint ("  first_inode = %d\n", data->sblock.first_inode); 
	BT_kPrint ("  inode_size = %d\n", data->sblock.inode_size); 
	BT_kPrint ("  block_group_number = %d\n", data->sblock.block_group_number); 
	BT_kPrint ("  feature_compatibility = %d\n", data->sblock.feature_compatibility); 
	BT_kPrint ("  feature_incompat = %d\n", data->sblock.feature_incompat); 
	BT_kPrint ("  feature_ro_compat = %d\n", data->sblock.feature_ro_compat); 
	BT_kPrint ("  unique_id = %08x%08x%08x%08x\n", data->sblock.unique_id[0], data->sblock.unique_id[1], data->sblock.unique_id[2], data->sblock.unique_id[3]); 
	BT_kPrint ("  volume_name = %s\n", data->sblock.volume_name); 
	BT_kPrint ("  last_mounted_on = %d\n", data->sblock.last_mounted_on); 
	BT_kPrint ("  compression_info = %d\n", data->sblock.compression_info); 
#endif

	/* Make sure this is an ext2 filesystem.  */
	if (bt_le16_to_cpu (data->sblock.magic) != EXT2_MAGIC) {
		goto fail;
	}

	data->diropen.data = data;
	data->diropen.ino = 2;
	data->diropen.inode_read = 1;
	data->inode = &data->diropen.inode;

	status = ext2fs_read_inode (data, 2, data->inode);
	if (status == 0) {
		goto fail;
	}
	
	ext2fs_root = data;

	return (0);

fail:
	BT_kPrint ("Failed to mount ext2 filesystem...\n");
	BT_kFree(data);
	ext2fs_root = NULL;
	return (-1);
}
