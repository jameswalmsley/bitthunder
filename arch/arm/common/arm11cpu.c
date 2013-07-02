#include <bitthunder.h>


BT_u32 BT_GetCoreID() {
	register BT_u32 retval asm("r0");

	__asm volatile("mrc p15,0,r0,c0,c0,5");
	__asm volatile("and r0, r0, #0xF" : "=r"(retval) : "r"(retval));

	return retval;
}

BT_u32 BT_GetTotalCores() {
	return BT_CONFIG_CPU_CORES;
}

//BT_u32 BT_SetCoreStartAddress
