#ifndef _MAC_H_
#define _MAC_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LM3Sxx_MAC_oDeviceInterface;

typedef struct _LM3Sxx_MAC_REGS {
	union {
		BT_u32	MACRIS;				// 0x00		MAC raw interrupt status register

#define LM3Sxx_MAC_MACRIS_PHYINT			0x00000040
#define LM3Sxx_MAC_MACRIS_MDINT			0x00000020
#define LM3Sxx_MAC_MACRIS_RXER				0x00000010
#define LM3Sxx_MAC_MACRIS_FOV				0x00000008
#define LM3Sxx_MAC_MACRIS_TXEMP			0x00000004
#define LM3Sxx_MAC_MACRIS_TXER				0x00000002
#define LM3Sxx_MAC_MACRIS_RXINT			0x00000001

		BT_u32	MACIACK;			// 0x00		MAC interrupt acknowledge register

#define LM3Sxx_MAC_MACIACK_PHYINT			0x00000040
#define LM3Sxx_MAC_MACIACK_MDINT			0x00000020
#define LM3Sxx_MAC_MACIACK_RXER			0x00000010
#define LM3Sxx_MAC_MACIACK_FOV				0x00000008
#define LM3Sxx_MAC_MACIACK_TXEMP			0x00000004
#define LM3Sxx_MAC_MACIACK_TXER			0x00000002
#define LM3Sxx_MAC_MACIACK_RXINT			0x00000001

	};
	BT_u32	MACIM;					// 0x04		MAC interrupt mask register

#define LM3Sxx_MAC_MACIM_PHYINT				0x00000040
#define LM3Sxx_MAC_MACIM_MDINT				0x00000020
#define LM3Sxx_MAC_MACIM_RXER				0x00000010
#define LM3Sxx_MAC_MACIM_FOV				0x00000008
#define LM3Sxx_MAC_MACIM_TXEMP				0x00000004
#define LM3Sxx_MAC_MACIM_TXER				0x00000002
#define LM3Sxx_MAC_MACIM_RXINT				0x00000001

	BT_u32	MACRCTL;				// 0x08		MAC receive control register

#define LM3Sxx_MAC_MACRCTL_RSTFIFO			0x00000010
#define LM3Sxx_MAC_MACRCTL_BADCRC			0x00000008
#define LM3Sxx_MAC_MACRCTL_PRMS			0x00000004
#define LM3Sxx_MAC_MACRCTL_AMUL			0x00000002
#define LM3Sxx_MAC_MACRCTL_RXEN			0x00000001

	BT_u32	MACTCTL;				// 0x0C		MAC transmit control register

#define LM3Sxx_MAC_MACTCTL_DUPLEX			0x00000010
#define LM3Sxx_MAC_MACTCTL_CRC				0x00000004
#define LM3Sxx_MAC_MACTCTL_PADEN			0x00000002
#define LM3Sxx_MAC_MACTCTL_TXEN			0x00000001

	BT_u32	MACDATA;				// 0x10		MAC data register
	BT_u32	MACIA0;					// 0x14		MAC individual address 0 register
	BT_u32	MACIA1;					// 0x18		MAC individual address 1 registers
	BT_u32	MACTHR;					// 0x1C		MAC threshold register
	BT_u32	MACMCTL;				// 0x20		MAC management control register

#define LM3Sxx_MAC_MACMCTL_REG(x)			((x << 3) & 0xF8)
#define LM3Sxx_MAC_MACMCTL_WRITE			0x00000002
#define LM3Sxx_MAC_MACMCTL_START			0x00000001

	BT_u32	MACMDV;					// 0x24		MAC management divider register
	BT_STRUCT_RESERVED_u32(0, 0x24, 0x2C);
	BT_u32	MACMTXD;				// 0x2C		MAC management transmit data register
	BT_u32	MACMRXD;				// 0x30		MAC management receive data register
	BT_u32	MACNP;					// 0x34		MAC number of packets register

#define	LM3Sxx_MAC_MACNP_M					0x0000003F

	BT_u32	MACTR;					// 0x38		MAC transmission request register

#define	LM3Sxx_MAC_MACTR_NEWTX				0x00000001

	BT_u32	MACTS;					// 0x3C

#define LM3Sxx_MAC_MACTS_TSEN				0x00000001

	BT_u32	MACLED;					// 0x40		MAC LED encoding register

#define	LM3Sxx_MAC_MACLED_LED1_LINK_ACTIVITY	0x00000800
#define	LM3Sxx_MAC_MACLED_LED1_FULL_DUPLEX		0x00000700
#define	LM3Sxx_MAC_MACLED_LED1_10_BASET		0x00000600
#define	LM3Sxx_MAC_MACLED_LED1_100_BASETX		0x00000500
#define	LM3Sxx_MAC_MACLED_LED1_ACTIVITY		0x00000100
#define	LM3Sxx_MAC_MACLED_LED1_LINK_OK			0x00000000

#define	LM3Sxx_MAC_MACLED_LED0_LINK_ACTIVITY	0x00000008
#define	LM3Sxx_MAC_MACLED_LED0_FULL_DUPLEX		0x00000007
#define	LM3Sxx_MAC_MACLED_LED0_10_BASET		0x00000006
#define	LM3Sxx_MAC_MACLED_LED0_100_BASETX		0x00000005
#define	LM3Sxx_MAC_MACLED_LED0_ACTIVITY		0x00000001
#define	LM3Sxx_MAC_MACLED_LED0_LINK_OK			0x00000000

	BT_u32	MDIX;					// 0x44		MAC PHY MDIX register

#define	LM3Sxx_MAC_MDIX_EN						0x00000001

} LM3Sxx_MAC_REGS;

#define MAC0						((LM3Sxx_MAC_REGS *) BT_CONFIG_MACH_LM3Sxx_MAC0_BASE)

#endif

