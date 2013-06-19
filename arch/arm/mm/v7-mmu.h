#ifndef _V7_MMU_H_
#define _V7_MMU_H_


#define	MMU_FAULT_MASK		0x00000003
#define MMU_FAULT			0x00000000

/**
 *	Defines the BitMask patterns for a section permissions.
 **/
#define MMU_SECTION_MASK	0x00040002
#define MMU_SECTION			0x00040002
#define MMU_SECTION_BASE	0xFFF00000
#define MMU_SECTION_NS		0x00080000	///< Shareable Bit.
#define MMU_SECTION_nG		0x00020000	///< Non-global section, e.g. Global and Process attributes.
#define MMU_SECTION_S		0x00010000
#define MMU_SECTION_AP_2	0x00008000
#define MMU_SECTION_TEX		0x00007000
#define MMU_SECTION_AP_1_0	0x00000C00
#define MMU_SECTION_IMP 	0x00000200
#define MMU_SECTION_DOMAIN	0x000001E0
#define MMU_SECTION_XN		0x00000010
#define MMU_SECTION_C		0x00000008
#define MMU_SECTION_B		0x00000004
#define MMU_SECTION_PXN		0x00000001

/**
 *	Defines the BitMask patterns required to point to a PageTable.
 **/
#define MMU_PT_MASK			0x00000003
#define MMU_PT				0x00000002
#define MMU_PT_BASE			0xFFFFFC00	///< Page table must appear at a 1Kb offset.
#define MMU_PT_IMP			0x00000200
#define MMU_PT_DOMAIN		0x000001E0
#define MMU_PT_SBZ			0x00000010
#define MMU_PT_NS			0x00000080	///< Non-secure bit.
#define MMU_PT_PXN			0x00000040	///< Execure Never bit.





#endif
