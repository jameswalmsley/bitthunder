#ifndef _BARRIER_H_
#define _BARRIER_H_

#include <bt_config.h>

#define nop()	__asm__ __volatile__ ("mov	r0, r0	@ nop");

#ifdef BT_CONFIG_ARCH_ARM_ARMv7
#define isb() __asm__ __volatile__ ("isb" : : : "memory")
#define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#endif

#ifdef BT_CONFIG_ARCH_ARM_ARMv6
#define isb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 4" : : "r" (0) : "memory")
#define dsb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")
#define dmb() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#endif




#endif
