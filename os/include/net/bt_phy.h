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

struct bt_phy_device {
	BT_HANDLE 				hMII;					///< Handle to MII bus driver.
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

	BT_HANDLE 				hMAC;

	void (*adjust_link)		(BT_HANDLE hMac);
	void (*adjust_state)	(BT_HANDLE hMac);
};

struct bt_mii_bus {
	BT_HANDLE 				hMAC;
	const BT_i8			   *name;
	BT_u8					id;
	void				   *mutex;
	struct bt_phy_device   *phy_map[32];	// List of all phys on the bus.
	BT_u32					phy_mask;
};

BT_ERROR BT_RegisterMiiBus(BT_HANDLE hMII, struct bt_mii_bus *bus);

#endif
