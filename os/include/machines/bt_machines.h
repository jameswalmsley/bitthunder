/**
 *	BitThunder Machine Description API.
 *
 **/
#ifndef _BT_MACHINES_H_
#define _BT_MACHINES_H_

#include "bt_config.h"
#include "bt_types.h"
#include "mach/bt_machine_types.h"
#include "interfaces/bt_interfaces.h"
#include "interrupts/bt_interrupts.h"
#include "devman/bt_integrated_device.h"
#include "devman/bt_integrated_driver.h"


typedef enum {
	BT_ARCH_NONE=0,
	BT_ARCH_ARM,
	BT_ARCH_PPC,
	BT_ARCH_MB,
} BT_MACH_ARCHITECTURE;

typedef struct _BT_MACHINE_DESCRIPTION {
	BT_MACH_ARCHITECTURE			eArchitecture;
	BT_MACH_TYPE					eMachType;
	const BT_i8				   	   *szpName;

	BT_u32							ulSystemClockHz;

	BT_u32							(*pfnGetCpuClockFrequency) ();
	BT_ERROR					   	(*pfnMachineInit)				(struct _BT_MACHINE_DESCRIPTION *pMachine);

	const BT_INTEGRATED_DEVICE	   *pInterruptController;
	const BT_INTEGRATED_DEVICE	   *pSystemTimer;
	const BT_INTEGRATED_DEVICE	   *pWatchdogTimer;

	const BT_IF_DEVICE		   	   *pBootLogger;				/// Pointer to device supporting chardev if.
	BT_u32							ulBootUartID;			/// Which instance number to use.
} BT_MACHINE_DESCRIPTION;



#define BT_MACHINE_START(_arch,_type,_name)				\
static const BT_MACHINE_DESCRIPTION __bt_mach_desc_##_type	\
BT_ATTRIBUTE_SECTION(".bt.arch.init") = {				\
	.eArchitecture 	= BT_ARCH_##_arch,					\
	.eMachType     	= BT_MACH_##_type,					\
	.szpName		= _name,



#define BT_MACHINE_END 									\
};


BT_u32 BT_GetCpuClockFrequency(void);

#endif
