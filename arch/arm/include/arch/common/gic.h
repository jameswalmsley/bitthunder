#ifndef _GIC_H_
#define _GIC_H_

#include <bt_struct.h>				///< Provides a convenient reserved section macro.

/**
 *	GIC CPU Register interface.
 *
 **/
typedef struct _GICC_REGS {
	BT_u32 	CTLR;					///< Control register, Enable (v1/v2) and security flags (v2).
	BT_u32	PMR;					///< Priority mask register.
	BT_u32	BPR;					///< Binary point register, where to split the group priority and subpriroty fields.
	BT_u32	IAR;					///< Interrupt acknowledge register. Contains the CPUID and InterruptID of the interrupt. Read acks!
	BT_u32	EOIR;					///< End of Interrupt reg, write to signal completed processing of the interrupt.
	BT_u32	RPR;					///< Running priority reg, indicates the running priority of the CPU interface.
	BT_u32	HPPIR;					///< Highest priority pending interrupt reg.
	BT_u32	ABPR;					///< Aliased binary point register. (For handling group 1 interrupts).
	BT_u32	AIAR;					///< v2. Interrupt acknowledge for group1.
	BT_u32	AEOIR;					///< v2. End of interrupt reg for group1.
	BT_u32	AHPPIR;

	BT_STRUCT_RESERVED_u32(0, 0x28, 0x40);

	// From 3C to include the section that would be there.
	BT_STRUCT_RESERVED_u32(1, 0x3C, 0xD0);	///< Could contain some implementation specific regs.

	BT_u32	APR[BT_STRUCT_ARRAY_u32(0xD0, 0xDC)];					///< v2. Active priority registers.

	//BT_STRUCT_RESERVED_u32(2, 0xDC, 0xE0);

	BT_u32	NSAPR[BT_STRUCT_ARRAY_u32(0xE0, 0xEC)];				///< v2. Non-secure Active priority registers.

	BT_STRUCT_RESERVED_u32(3, 0xEC, 0xFC);

	BT_u32	IIDR;					///< CPU interface identification register.

	BT_STRUCT_RESERVED_u32(4, 0xFC, 0x1000);

	BT_u32	DIR;					///< v2. Deactivate Interrupt register.
} GICC_REGS;


typedef struct _GICD_REGS {
	BT_u32	CTLR;
	BT_u32	TYPER;
	BT_u32	IIDR;

	BT_STRUCT_RESERVED_u32(0, 0x08, 0x80);					///< There could be some implementation specific registers within this range.

	BT_u32	IGROUP[BT_STRUCT_ARRAY_u32(0x80, 0x0FC)];		///< Interrupt Group register (Security feature).

	BT_STRUCT_RESERVED_u32(1, 0xFC, 0x100);

	BT_u32	ISENABLER[BT_STRUCT_ARRAY_u32(0x100, 0x17c)];	///< Interrupt set-enable registers.

	BT_STRUCT_RESERVED_u32(2, 0x17C, 0x180);

	BT_u32	ICENABLER[BT_STRUCT_ARRAY_u32(0x180, 0x1FC)];

	BT_STRUCT_RESERVED_u32(3, 0x1FC, 0x200);

	BT_u32 	ISPENDR[BT_STRUCT_ARRAY_u32(0x200, 0x27C)];



} GICD_REGS;

const BT_INTERRUPT_CONTROLLER BT_ARM_GIC_oInterface;

#endif
