#ifndef _IBM_MBR_H_
#define _IBM_MBR_H_

#define IBM_MBR_BYTES_PER_SECTOR	0x00B
#define IBM_MBR_PTBL				0x1BE
#define IBM_MBR_PTBL_ACTIVE			0x000
#define IBM_MBR_PTBL_ID				0x004
#define IBM_MBR_PTBL_LBA			0x008
#define IBM_MBR_PTBL_SECTORS		0x00C

#define IBM_MBR_SIGNATURE        	0x1FE

/*=========================================================================================== */

#define	OFS_PART_ACTIVE_8             0x000 /* 0x01BE 0x80 if active */
#define	OFS_PART_START_HEAD_8         0x001 /* 0x01BF */
#define	OFS_PART_START_SEC_TRACK_16   0x002 /* 0x01C0 */
#define	OFS_PART_ID_NUMBER_8          0x004 /* 0x01C2 */
#define	OFS_PART_ENDING_HEAD_8        0x005 /* 0x01C3 */
#define	OFS_PART_ENDING_SEC_TRACK_16  0x006 /* 0x01C4   = SectorCount - 1 - ulHiddenSectors */
#define	OFS_PART_STARTING_LBA_32      0x008 /* 0x01C6   = ulHiddenSectors (This is important) */
#define	OFS_PART_LENGTH_32            0x00C /* 0x01CA   = SectorCount - 1 - ulHiddenSectors */

#define	OFS_PTABLE_MACH_CODE          0x000 /* 0x0000 */
#define	OFS_PTABLE_PART_0             0x1BE /* 446 */
#define	OFS_PTABLE_PART_1             0x1CE /* 462 */
#define	OFS_PTABLE_PART_2             0x1DE /* 478 */
#define	OFS_PTABLE_PART_3             0x1FE /* 494 */
#define	OFS_PTABLE_PART_LEN           16

/*=========================================================================================== */

#define	OFS_BPB_jmpBoot_24           0x000 /* uchar jmpBoot[3] "0xEB 0x00 0x90" */
#define	OFS_BPB_OEMName_64           0x003 /* uchar BS_OEMName[8] "MSWIN4.1" */

#define	OFS_BPB_BytsPerSec_16        0x00B /* Only 512, 1024, 2048 or 4096 */
#define	OFS_BPB_SecPerClus_8         0x00D /* Only 1, 2, 4, 8, 16, 32, 64, 128 */
#define	OFS_BPB_ResvdSecCnt_16       0x00E /* ulFATReservedSectors, e.g. 1 (FAT12/16) or 32 (FAT32) */

#define	OFS_BPB_NumFATs_8            0x010 /* 2 recommended */
#define	OFS_BPB_RootEntCnt_16        0x011 /* ((iFAT16RootSectors * 512) / 32) 512 (FAT12/16) or 0 (FAT32) */
#define	OFS_BPB_TotSec16_16          0x013 /* xxx (FAT12/16) or 0 (FAT32) */
#define	OFS_BPB_Media_8              0x015 /* 0xF0 (rem media) also in FAT[0] low byte */

#define	OFS_BPB_FATSz16_16           0x016
#define	OFS_BPB_SecPerTrk_16         0x018 /* n.a. CF has no tracks */
#define	OFS_BPB_NumHeads_16          0x01A /* n.a. 1 ? */
#define	OFS_BPB_HiddSec_32           0x01C /* n.a.	0 for nonparitioned volume */
#define	OFS_BPB_TotSec32_32          0x020 /* >= 0x10000 */

#define	OFS_BPB_16_DrvNum_8          0x024 /* n.a. */
#define	OFS_BPB_16_Reserved1_8       0x025 /* n.a. */
#define	OFS_BPB_16_BootSig_8         0x026 /* n.a. */
#define	OFS_BPB_16_BS_VolID_32       0x027 /* "unique" number */
#define	OFS_BPB_16_BS_VolLab_88      0x02B /* "NO NAME    " */
#define	OFS_BPB_16_FilSysType_64     0x036 /* "FAT12   " */

#define	OFS_BPB_32_FATSz32_32        0x024 /* Only when BPB_FATSz16 = 0 */
#define	OFS_BPB_32_ExtFlags_16       0x028 /* FAT32 only */
#define	OFS_BPB_32_FSVer_16          0x02A /* 0:0 */
#define	OFS_BPB_32_RootClus_32       0x02C /* See 'iFAT32RootClusters' Normally 2 */
#define	OFS_BPB_32_FSInfo_16         0x030 /* Normally 1 */
#define	OFS_BPB_32_BkBootSec_16      0x032 /* Normally 6 */
#define	OFS_BPB_32_Reserved_96       0x034 /* Zeros */
#define	OFS_BPB_32_DrvNum_8          0x040 /* n.a. */
#define	OFS_BPB_32_Reserved1_8       0x041 /* n.a. */
#define	OFS_BPB_32_BootSig_8         0x042 /* n.a. */
#define	OFS_BPB_32_VolID_32          0x043 /* "unique" number */
#define	OFS_BPB_32_VolLab_88         0x047 /* "NO NAME    " */
#define	OFS_BPB_32_FilSysType_64     0x052 /* "FAT12   " */

#define	OFS_FSI_32_LeadSig			0x000 /* With contents 0x41615252 */
#define	OFS_FSI_32_Reserved1		0x004 /* 480 times 0 */
#define	OFS_FSI_32_StrucSig			0x1E4 /* With contents 0x61417272 */
#define	OFS_FSI_32_Free_Count		0x1E8 /* last known free cluster count on the volume, ~0 for unknown */
#define	OFS_FSI_32_Nxt_Free			0x1EC /* cluster number at which the driver should start looking for free clusters */
#define	OFS_FSI_32_Reserved2		0x1F0 /* zero's */
#define	OFS_FSI_32_TrailSig			0x1FC /* 0xAA550000 (little endian) */


#endif
