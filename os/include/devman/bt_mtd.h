/*
 * bt_mtd.h
 *
 *  Created on: 05.11.2013
 *      Author: dq
 */

#ifndef BT_MTD_H_
#define BT_MTD_H_

#include <fs/bt_devfs.h>
#include <collections/bt_list.h>

#define BT_MTD_ERASE_PENDING	0x01
#define BT_MTD_ERASING			0x02
#define BT_MTD_ERASE_SUSPEND	0x04
#define BT_MTD_ERASE_DONE		0x08
#define BT_MTD_ERASE_FAILED		0x10

#define BT_MTD_FAIL_ADDR_UNKNOWN -1LL

#define BT_MTD_ABSENT			0
#define BT_MTD_RAM				1
#define BT_MTD_ROM				2
#define BT_MTD_NORFLASH			3
#define BT_MTD_NANDFLASH		4
#define BT_MTD_DATAFLASH		6
#define BT_MTD_UBIVOLUME		7
#define BT_MTD_MLCNANDFLASH		8

#define BT_MTD_WRITEABLE		0x400	/* Device is writeable */
#define BT_MTD_BIT_WRITEABLE	0x800	/* Single bits can be flipped */
#define BT_MTD_NO_ERASE			0x1000	/* No erase necessary */
#define BT_MTD_POWERUP_LOCK		0x2000	/* Always locked after reset */

/* Some common devices / combinations of capabilities */
#define BT_MTD_CAP_ROM			0
#define BT_MTD_CAP_RAM			(BT_MTD_WRITEABLE | BT_MTD_BIT_WRITEABLE | BT_MTD_NO_ERASE)
#define BT_MTD_CAP_NORFLASH		(BT_MTD_WRITEABLE | BT_MTD_BIT_WRITEABLE)
#define BT_MTD_CAP_NANDFLASH	(BT_MTD_WRITEABLE)

/* Obsolete ECC byte placement modes (used with obsolete MEMGETOOBSEL) */
#define BT_MTD_NANDECC_OFF			0	// Switch off ECC (Not recommended)
#define BT_MTD_NANDECC_PLACE		1	// Use the given placement in the structure (YAFFS1 legacy mode)
#define BT_MTD_NANDECC_AUTOPLACE	2	// Use the default placement scheme
#define BT_MTD_NANDECC_PLACEONLY	3	// Use the given placement in the structure (Do not store ecc result on read)
#define BT_MTD_NANDECC_AUTOPL_USR 	4	// Use the given autoplacement scheme rather than using the default

/* OTP mode selection */
#define BT_MTD_OTP_OFF			0
#define BT_MTD_OTP_FACTORY		1
#define BT_MTD_OTP_USER			2

typedef struct _BT_MTD_ERASE_REGION_INFO {
	BT_u64			offset;			/* At which this region starts, from the beginning of the MTD */
	BT_u32			erasesize;		/* For this region */
	BT_u32			numblocks;		/* Number of blocks of erasesize in this region */
} BT_MTD_ERASE_REGION_INFO;

typedef struct _BT_MTD_INFO {
	BT_HANDLE_HEADER h;
	struct bt_list_head item;
	BT_u32 			offset;
	BT_HANDLE		 hMtd;
	struct bt_devfs_node node;
	BT_u32			ulReferenceCount;
	BT_u8			 type;
	BT_u32			 flags;
	BT_u64			 size;			// Total size of the MTD

	/* "Major" erase size for the device. Naive users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	BT_u32			 erasesize;
	/* Minimal writable flash unit size. In case of NOR flash it is 1 (even
	 * though individual bits can be cleared), in case of NAND flash it is
	 * one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
	 * it is of ECC block size, etc. It is illegal to have writesize = 0.
	 * Any driver registering a struct mtd_info must ensure a writesize of
	 * 1 or larger.
	 */
	BT_u32			writesize;

	/*
	 * Size of the write buffer used by the MTD. MTD devices having a write
	 * buffer can write multiple writesize chunks at a time. E.g. while
	 * writing 4 * writesize bytes to a device with 2 * writesize bytes
	 * buffer the MTD driver can (but doesn't have to) do 2 writesize
	 * operations, but not 4. Currently, all NANDs have writebufsize
	 * equivalent to writesize (NAND page size). Some NOR flashes do have
	 * writebufsize greater than writesize.
	 */
	BT_u32 			 writebufsize;

	BT_u32 			 oobsize;   // Amount of OOB data per block (e.g. 16)
	BT_u32 			 oobavail;  // Available OOB bytes per block

	/*
	 * If erasesize is a power of 2 then the shift is stored in
	 * erasesize_shift otherwise erasesize_shift is zero. Ditto writesize.
	 */
	BT_u32 			 erasesize_shift;
	BT_u32 			 writesize_shift;
	/* Masks based on erasesize_shift and writesize_shift */
	BT_u32 			 erasesize_mask;
	BT_u32 			 writesize_mask;

	const char *name;
	int numeraseregions;
	BT_MTD_ERASE_REGION_INFO *eraseregions;
} BT_MTD_INFO;


typedef struct {
	BT_u64			 size;			// Total size of the MTD
	BT_u32			 erasesize;
	BT_u8			 type;
	BT_u32			 flags;
} BT_MTD_USER_INFO;

/*
 *	Define the unified API for SPI devices in BitThunder
 */

void mtd_erase_callback(BT_HANDLE hFlash, BT_MTD_ERASE_INFO *instr);

BT_ERROR BT_MTD_RegisterDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_MTD_INFO *mtd);
BT_ERROR BT_MTD_Erase(BT_HANDLE hMTD, BT_MTD_ERASE_INFO *instr);
BT_ERROR BT_MTD_Read(BT_HANDLE hMTD, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf);
BT_ERROR BT_MTD_Write(BT_HANDLE hMTD, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf);
BT_ERROR BT_MTD_GetUserInfo(BT_HANDLE hMTD, BT_MTD_USER_INFO * info);

#endif /* BT_MTD_H_ */
