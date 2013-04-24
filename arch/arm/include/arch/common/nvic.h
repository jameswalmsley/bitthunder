#ifndef _NVIC_H_
#define _NVIC_H_

#include <bt_struct.h>

typedef struct _NVIC_REGS {
	BT_u32 	ISE[8];
	BT_STRUCT_RESERVED_u32(0, 0x11C, 0x180);
	BT_u32 	ICE[8];
	BT_STRUCT_RESERVED_u32(1, 0x19C, 0x200);
	BT_u32	ISP[8];
	BT_STRUCT_RESERVED_u32(2, 0x21C, 0x280);
	BT_u32	ICP[8];
	BT_STRUCT_RESERVED_u32(3, 0x29C, 0x300);
	BT_u32	IAB[8];
	BT_STRUCT_RESERVED_u32(4, 0x31C, 0x400);
	BT_u8	IP[256];
	BT_STRUCT_RESERVED_u32(5, 0x500, 0xF00);
	BT_u32	STIR;
} NVIC_REGS;






#endif
