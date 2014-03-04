/**
 *	Network PHY support for BitThunder.
 *
 **/

#ifndef _BT_PHY_H_
#define _BT_PHY_H_

enum bt_phy_state {
	PHY_DOWN=0,
	PHY_STARTING,
	PHY_READY,
	PHY_PENDING,
	PHY_UP,
	PHY_AN,
	PHY_RUNNING,
	PHY_NOLINK,
	PHY_FORCING,
	PHY_CHANGELINK,
	PHY_HALTED,
	PHY_RESUMING
};

enum bt_phy_interface_t {
	BT_PHY_INTERFACE_MODE_NA,
	BT_PHY_INTERFACE_MODE_MII,
	BT_PHY_INTERFACE_MODE_GMII,
	BT_PHY_INTERFACE_MODE_SGMII,
	BT_PHY_INTERFACE_MODE_TBI,
	BT_PHY_INTERFACE_MODE_RMII,
	BT_PHY_INTERFACE_MODE_RGMII,
	BT_PHY_INTERFACE_MODE_RGMII_ID,
	BT_PHY_INTERFACE_MODE_RGMII_RXID,
	BT_PHY_INTERFACE_MODE_RGMII_TXID,
	BT_PHY_INTERFACE_MODE_RTBI,
	BT_PHY_INTERFACE_MODE_SMII,
} bt_phy_interface_t;

struct bt_mii_bus;

struct bt_phy_device {
	struct bt_mii_bus 	   *mii_bus;
	struct bt_list_head		item;
	BT_HANDLE				hPHY;					///< PHY driver handle.
	BT_u32					phy_id;					///< UID found during probe.
	enum bt_phy_state		eState;
	enum bt_phy_interface_t	interface;
	BT_u32					addr;					///< Phy bus address.
	BT_u32					speed;
	BT_u32					duplex;
	BT_u32					pause;
	BT_u32					asym_pause;
	BT_u32					link;
	BT_u32					interrupts;
	BT_u32					supported;
	BT_u32					advertising;
	BT_u32					autoneg;
	BT_u32					link_timeout;
	BT_s32					irq;
	BT_HANDLE				active_mac;				///< Handle of the associated/connected MAC.
};

struct bt_mii_bus {
	BT_HANDLE 				hMAC;
	BT_HANDLE 				hMII;			///< Handle to MII bus driver.
	struct bt_list_head		item;
	struct bt_list_head		phys;			// Phy devices on this bus.
	const BT_i8			   *name;
	BT_u8					id;
	void				   *mutex;
	struct bt_phy_device   *phy_map[32];	// List of all phys on the bus.
	BT_u32					phy_mask;
};

BT_ERROR BT_RegisterMiiBus(BT_HANDLE hMII, struct bt_mii_bus *bus);

BT_u16 bt_phy_read(struct bt_phy_device *phy, BT_u32 regnum, BT_ERROR *pError);
BT_ERROR bt_phy_write(struct bt_phy_device *phy, BT_u32 regnum, BT_u16 val);

/*
 *	Standard PHY Register definitions.
 *
 *	The following definitions describe the standard PHY register layout.
 *	All good PHYs should implement this scheme.
 */

/*
 *	Register 0 : Copper Control Register
 */
#define BT_PHY_CCR_COPPER_RESET 			0x8000
#define BT_PHY_CCR_LOOPBACK 				0x4000
#define BT_PHY_CCR_SPEED_SELECT_LSB			0x2000
#define BT_PHY_CCR_AUTONEG_ENABLE 			0x1000
#define BT_PHY_CCR_POWER_DOWN 				0x0800
#define BT_PHY_CCR_ISOLATE 					0x0400
#define BT_PHY_CCR_RESTART_AUTONEG 			0x0200
#define BT_PHY_CCR_COPPER_DUPLEX_MODE 		0x0100
#define BT_PHY_CCR_COLLISION_TEST 			0x0080
#define BT_PHY_CCR_SPEED_SELECT_MSB 		0x0040
#define BT_PHY_CCR_RESERVED_0 				0x001F

/*
 *	Register 1 : Copper Status Register
 */
#define BT_PHY_CSR_100BASE_T4				0x8000		///< 0 = PHY not able to perform 100BASE-T4
#define BT_PHY_CSR_100BASE_X_FULL_DUPLEX	0x4000		///< 1 = PHY able to perform full-duplex 100BASE-X
#define BT_PHY_CSR_100BASE_X_HALF_DUPLEX 	0x2000		///< 1 = PHY able to perform half-duplex 100BASE-X
#define BT_PHY_CSR_10MBPS_FULL_DUPLEX 		0x1000		///< 1 = PHY able to perform full-duplex 10BASE-T
#define BT_PHY_CSR_10MBPS_HALF_DUPLEX		0x0800		///< 1 = PHY able to perform half-duplex 10BASE-T
#define BT_PHY_CSR_100BASE_T2_FULL_DUPLEX 	0x0400		///< 0 = PHY not able to perform full duplex.
#define BT_PHY_CSR_100BASE_T2_HALF_DUPLEX 	0x0200		///< 0 = PHY not able to perform half duplex.
#define BT_PHY_CSR_EXTENDED_STATUS 			0x0100		///< 1 = Extended status information in Register 15.
#define BT_PHY_CSR_RESERVED_0 				0x0080		///< Always read as 0.
#define BT_PHY_CSR_MF_PREAMBLE_SUPPRESSION 	0x0040		///< 1 = PHY accepts management frames with preamble suppressed.
#define BT_PHY_CSR_COPPER_AUTONEG_COMPLETE 	0x0020		///< 1 = Auto-negotiation process complete.
#define BT_PHY_CSR_COPPER_REMOTE_FAULT 		0x0010		///< 1 = Remote fault condition detected.
#define BT_PHY_CSR_COPPER_AUTONEG_ABILITY 	0x0008		///< 1 = PHY able to perform auto-negotiation.
#define BT_PHY_CSR_COPPER_LINK_STATUS 		0x0004		///< 1 = link is up.
#define BT_PHY_CSR_JABBER_DETECT 			0x0002		///< 1 = Jabber condition detected.
#define BT_PHY_CSR_EXTENDED_CAPABILITY 		0x0001		///< 1 = Extended register capabilities.

#endif
