#ifndef _BT_MACHINES_H_
#define _BT_MACHINES_H_

#include "bt_config.h"
#include "bt_types.h"


typedef enum {
	BT_ARCH_NONE=0,
	BT_ARCH_ARM,
	BT_ARCH_PPC,
	BT_ARCH_MB,
} BT_MACHINE_ARCHITECTURE;


typedef struct _BT_MACHINE_DESCRIPTION {
	BT_MACHINE_ARCHITECTURE		eArchitecture;
	BT_u32						ulMachID;
	const BT_i8				   *szpName;

	BT_u32						ulTotalIRQs;

	// Integrated IRQ Interface

	// Integrated Timer Interface

} BT_MACHINE_DESCRIPTION;



#define BT_MACHINE_START(_arch,_type,_name)				\
static const BT_MACHINE_DESCRIPTION __mach_desc##_type	\
BT_ATTRIBUTE_SECTION(".bt.arch.init") = {				\
	.eArchitecture 	= BT_ARCH_##_arch,					\
	.ulMachID      	= _type,							\
	.szpName		= _name,
	  


#define BT_MACHINE_END 									\
};







#endif
