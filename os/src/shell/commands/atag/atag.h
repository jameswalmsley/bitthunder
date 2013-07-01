/**
 *	ATAG structure definition.
 *
 *
 **/

#include <bt_types.h>

#define ATAG_NONE       0x00000000
#define ATAG_CORE       0x54410001
#define ATAG_MEM        0x54410002
#define ATAG_VIDEOTEXT  0x54410003
#define ATAG_RAMDISK    0x54410004
#define ATAG_INITRD2    0x54420005
#define ATAG_SERIAL     0x54410006
#define ATAG_REVISION   0x54410007
#define ATAG_VIDEOLFB   0x54410008
#define ATAG_CMDLINE    0x54410009

struct atag_header {
	BT_u32 	size;		// Length of TAG in words, including this header.
	BT_u32 	tag;		// Tag type ID.
};

struct atag_core {
	BT_u32 	flags;
	BT_u32	pagesize;
	BT_u32	rootdev;
};

struct atag_mem {
	BT_u32	size;
	BT_u32	start;
};

struct atag_videotext {
	BT_u8	x;
	BT_u8	y;
	BT_u16	vide_page;
	BT_u8	video_mode;
	BT_u8	video_cols;
	BT_u16	video_ega_bx;
	BT_u8	video_lines;
	BT_u8	video_isvga;
	BT_u16	video_points;
};

struct atag_ramdisk {
	BT_u32	flags;
	BT_u32	size;
	BT_u32 	start;
};

struct atag_initrd2 {
	BT_u32	start;
	BT_u32	size;
};

struct atag_serialnr {
	BT_u32 	low;
	BT_u32	high;
};

struct atag_revision {
	BT_u32 	rev;
};

struct atag_videolfb {
	BT_u16	lfb_width;
	BT_u16  lfb_height;
	BT_u16	lfb_depth;
	BT_u16	lfb_linelength;
	BT_u32	lfb_base;
	BT_u32	lfb_size;
	BT_u8	red_size;
	BT_u8	red_pos;
	BT_u8	green_size;
	BT_u8	green_pos;
	BT_u8	blue_size;
	BT_u8	blue_pos;
	BT_u8	rsvd_size;
	BT_u8	rsvd_pos;
};

struct atag_cmdline {
	BT_i8	*cmdline;
};

struct atag {
	struct atag_header hdr;
	union {
		struct atag_core		core;
		struct atag_mem			mem;
		struct atag_videotext	videotext;
		struct atag_ramdisk		ramdisk;
		struct atag_initrd2		initrd2;
		struct atag_serialnr	serialnr;
		struct atag_revision	revision;
		struct atag_videolfb	videolfb;
		struct atag_cmdline		cmdline;
	} u;
};

#define tag_next(t)		((struct atag *) ((BT_u32 *)(t) + (t)->hdr.size))
#define tag_size(type)	((sizeof(struct atag_header) + sizeof(struct type)) >> 2)
