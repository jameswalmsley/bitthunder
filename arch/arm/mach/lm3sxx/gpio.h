#ifndef _GPIO_H_
#define _GPIO_H_

#include <bitthunder.h>

typedef struct _LM3Sxx_GPIO_BANK {
	BT_u32 DATA[256];
	BT_u32 DIR;
	BT_u32 IS;
	BT_u32 IBE;
	BT_u32 IEV;
	BT_u32 IM;
	BT_u32 RIS;
	BT_u32 MIS;
	BT_u32 ICR;
	BT_u32 AFSEL;
	BT_STRUCT_RESERVED_u32(0, 0x420, 0x500);
	BT_u32 DR2R;
	BT_u32 DR4R;
	BT_u32 DR8R;
	BT_u32 ODR;
	BT_u32 PUR;
	BT_u32 PDR;
	BT_u32 SLR;
	BT_u32 DEN;
	BT_u32 LOCK;
	BT_u32 CR;
	BT_u32 AMSEL;
	BT_u32 PCTL;
	BT_STRUCT_RESERVED_u32(1, 0x52C, 0xFD0);
	BT_u32 PeriphID[8];
	BT_u32 CellID[4];
} LM3Sxx_GPIO_BANK;

typedef struct _LM3Sxx_GPIO_REGS {
	LM3Sxx_GPIO_BANK banks[BT_CONFIG_MACH_LM3Sxx_TOTAL_GPIOS/8];
} LM3Sxx_GPIO_REGS;


#endif
