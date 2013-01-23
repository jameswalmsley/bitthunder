/**
 *	This file defines all system types for BitThunder.
 **/
#ifndef _BT_TYPES_H_
#define _BT_TYPES_H_

#include "bt_arch_types.h"							///< Platform dependent primary types.

typedef BT_u32				BT_ERROR;

typedef BT_u32				BT_BOOL;

#define BT_FALSE			0
#define BT_TRUE				1


struct _BT_OPAQUE_HANDLE;
typedef struct _BT_OPAQUE_HANDLE *BT_HANDLE;


typedef struct {
#ifdef BT_CONFIG_ARCH_LITTLE_ENDIAN
	BT_u8	a,b,c,d;
#endif
#ifdef BT_CONFIG_ARCH_BIG_ENDIAN
	BT_u8	d,c,b,a;
#endif
} BT_IPPARTS;

typedef union {
	BT_u32		ulIPAddress;
	BT_IPPARTS 	oParts;
} BT_IPADDRESS;

/**
 *	Allows compiler independent casting of a pointer type!
 **/
typedef union _BT_POINTER {
	void   *p;
	BT_u64 	u64;
} BT_POINTER;

typedef BT_u32	BT_TICK;

#ifndef NULL
#define NULL	((void *) 0)
#endif

#endif
