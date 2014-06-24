/**
 *	Network PHY support for BitThunder.
 *
 *
 **/

#ifndef _BT_PHY_H_
#define _BT_PHY_H_

enum bt_phy_state {
	BT_PHY_DOWN=0,
	BT_PHY_STARTING,
	BT_PHY_READY,
	BT_PHY_PENDING,
	BT_PHY_UP,
	BT_PHY_AN,
	BT_PHY_RUNNING,
	BT_PHY_NOLINK,
	BT_PHY_FORCING,
	BT_PHY_RENEGOTIATE,
	BT_PHY_CHANGELINK,
	BT_PHY_HALTED,
	BT_PHY_RESUMING
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
};

struct bt_mii_bus;

#define BT_PHY_SUPPORTED_10baseT_Half 		(1 << 0)		///< PHY supports 10base-T at half-duplex.
#define BT_PHY_SUPPORTED_10baseT_Full		(1 << 1)		///< PHY supports 10base-T at full-duplex.
#define BT_PHY_SUPPORTED_100baseT_Half	   	(1 << 2)		///< PHY supports 100base-T at half-duplex.
#define BT_PHY_SUPPORTED_100baseT_Full		(1 << 3)		///< PHY supports 100base-T at full-duplex.
#define BT_PHY_SUPPORTED_1000baseT_Half 	(1 << 4)		///< PHY supports 1000base-T at half-duplex.
#define BT_PHY_SUPPORTED_1000baseT_Full 	(1 << 5)		///< PHY supports 1000base-T at full-duplex.
#define BT_PHY_SUPPORTED_Autoneg 			(1 << 6)		///< PHY supports automatic link negotiation.
#define BT_PHY_SUPPORTED_TP 				(1 << 7)
#define BT_PHY_SUPPORTED_AUI 				(1 << 8)
#define BT_PHY_SUPPORTED_MII 				(1 << 9)
#define BT_PHY_SUPPORTED_FIBRE 				(1 << 10)
#define BT_PHY_SUPPORTED_BNC 				(1 << 11)
#define BT_PHY_SUPPORTED_10000baseT_Full 	(1 << 12)		///< PHY supports 10000base-T at full duplex.
#define BT_PHY_SUPPORTED_Pause 				(1 << 13)		///< PHY supports pause frames.
#define BT_PHY_SUPPORTED_Asym_Pause 		(1 << 14)		///< PHY supports asymetric pause frames.

#define BT_PHY_DUPLEX_HALF	0
#define BT_PHY_DUPLEX_FULL	1

struct bt_phy_linkstate {
	BT_u32	link;
	BT_u32	speed;
	BT_u32 	duplex;
	BT_u32	supported;
	BT_u32 	advertising;
	BT_u32	autoneg;
};

struct bt_phy_config {
	BT_u32	advertising_disable;					///< Mask of modes and features not to be advertised.
};

struct bt_phy_device {
	struct bt_mii_bus 	   *mii_bus;				///< Link back to the mii_bus used to communicate with this PHY.
	struct bt_list_head		item;
	BT_HANDLE				hPHY;					///< PHY driver handle.
	BT_u32					phy_id;					///< UID found during probe.
	enum bt_phy_state		eState;					///< Current state of the PHY state maching for this PHY.
	enum bt_phy_interface_t	interface;
	BT_u32					addr;					///< Phy bus address.
	BT_u32					speed;					///< Speed of link in MBit (10/100/1000/10000).
	BT_u32					duplex;					///< 0 = Half, 1 = Full
	BT_u32					pause;					///< 0 = Pause frames disabled, 1 = Pause frames enabled.
	BT_u32					asym_pause;				///< 0 = Asymetric pause frames disabled, 1 = Asymetric pause frames enabled.
	BT_u32					link;					///< 0 = Link "down", 1 = Link "up".
	BT_u32					interrupts;
	BT_u32					supported;				///< Mask of link modes and features that the PHY supports.
	BT_u32					advertising;			///< Mask of link modes and features that the PHY is advertising.
	BT_u32					advertising_mask;		///< Mask of link modes and features that should be disabled during negotiation.
	BT_u32					autoneg;				///< 0 = Autoneg disabled, 1 = Autoneg enabled.
	BT_u32					link_timeout;
	BT_s32					irq;					///< -1 = Polling mode, else IRQ number PHY uses for service requests.
	BT_HANDLE				active_mac;				///< Handle of the associated/connected MAC.
	const BT_DEVICE		   *pDevice;
};

struct bt_mii_bus {
	BT_HANDLE 				hMAC;
	BT_HANDLE 				hMII;			///< Handle to MII bus driver.
	const BT_DEVICE 	   *pDevice;		///< Bus device object, provides a reference into the device tree.
	struct bt_list_head		item;
	struct bt_list_head		phys;			///< List of PHY devices on this bus.
	const BT_i8			   *name;
	BT_u8					id;
	void				   *mutex;			///< Mutex for locking the MII bus.
	//struct bt_phy_device   *phy_map[32];	///< List of all phys on the bus.
	//BT_u32					phy_mask;
};

BT_ERROR BT_RegisterMiiBus(BT_HANDLE hMII, struct bt_mii_bus *bus);
BT_ERROR BT_ConnectPHY(BT_HANDLE hMac, BT_u32 ulAddress);

/*
 *	Kernel Internal PHY access API.
 */
BT_u16 bt_phy_read(struct bt_phy_device *phy, BT_u32 regnum, BT_ERROR *pError);
BT_ERROR bt_phy_write(struct bt_phy_device *phy, BT_u32 regnum, BT_u16 val);


/*
 *	Generic PHY implementation methods:
 */
BT_ERROR	bt_phy_generic_init(struct bt_phy_device *phy);
BT_ERROR 	bt_phy_generic_read_status(struct bt_phy_device *phy);

/*
 *	Standard PHY Register definitions.
 *
 *	The following definitions describe the standard PHY register layout.
 *	All good PHYs should implement this scheme.
 */

#define BT_PHY_MII_BMCR 					0x0
#define BT_PHY_MII_BMSR 					0x1
#define BT_PHY_MII_PHYSID1 					0x2
#define BT_PHY_MII_PHYSID2 					0x3
#define BT_PHY_MII_ADVERTISE				0x4
#define BT_PHY_MII_LPA 						0x5
#define BT_PHY_MII_EXPANSION 				0x6
#define BT_PHY_MII_CTRL1000 				0x9
#define BT_PHY_MII_STAT1000 				0xa
#define BT_PHY_MII_ESTATUS 					0xf

/*
 *	Register 0 : Basic Mode Control Register
 */
#define BT_PHY_BMCR_RESET					0x8000
#define BT_PHY_BMCR_LOOPBACK				0x4000
#define BT_PHY_BMCR_SPEED_SELECT_LSB		0x2000
#define BT_PHY_BMCR_AUTONEG_ENABLE			0x1000
#define BT_PHY_BMCR_POWER_DOWN				0x0800
#define BT_PHY_BMCR_ISOLATE					0x0400
#define BT_PHY_BMCR_RESTART_AUTONEG			0x0200
#define BT_PHY_BMCR_COPPER_DUPLEX_MODE		0x0100
#define BT_PHY_BMCR_COLLISION_TEST			0x0080
#define BT_PHY_BMCR_SPEED_SELECT_MSB		0x0040
#define BT_PHY_BMCR_RESERVED_0				0x003F

/*
 *	Register 1 : Basic Mode Status Register
 */
#define BT_PHY_BMSR_100BASE_T4				0x8000		///< 0 = PHY not able to perform 100BASE-T4
#define BT_PHY_BMSR_100BASE_X_FULL_DUPLEX	0x4000		///< 1 = PHY able to perform full-duplex 100BASE-X
#define BT_PHY_BMSR_100BASE_X_HALF_DUPLEX	0x2000		///< 1 = PHY able to perform half-duplex 100BASE-X
#define BT_PHY_BMSR_10MBPS_FULL_DUPLEX		0x1000		///< 1 = PHY able to perform full-duplex 10BASE-T
#define BT_PHY_BMSR_10MBPS_HALF_DUPLEX		0x0800		///< 1 = PHY able to perform half-duplex 10BASE-T
#define BT_PHY_BMSR_100BASE_T2_FULL_DUPLEX	0x0400		///< 0 = PHY not able to perform full duplex.
#define BT_PHY_BMSR_100BASE_T2_HALF_DUPLEX	0x0200		///< 0 = PHY not able to perform half duplex.
#define BT_PHY_BMSR_EXTENDED_STATUS			0x0100		///< 1 = Extended status information in Register 15.
#define BT_PHY_BMSR_RESERVED_0				0x0080		///< Always read as 0.
#define BT_PHY_BMSR_MF_PREAMBLE_SUPPRESSION	0x0040		///< 1 = PHY accepts management frames with preamble suppressed.
#define BT_PHY_BMSR_AUTONEG_COMPLETE		0x0020		///< 1 = Auto-negotiation process complete.
#define BT_PHY_BMSR_REMOTE_FAULT			0x0010		///< 1 = Remote fault condition detected.
#define BT_PHY_BMSR_AUTONEG_ABILITY			0x0008		///< 1 = PHY able to perform auto-negotiation.
#define BT_PHY_BMSR_LINK_STATUS				0x0004		///< 1 = link is up.
#define BT_PHY_BMSR_JABBER_DETECT			0x0002		///< 1 = Jabber condition detected.
#define BT_PHY_BMSR_EXTENDED_CAPABILITY		0x0001		///< 1 = Extended register capabilities.

#define BT_PHY_ESTATUS_1000_TFULL			0x2000
#define BT_PHY_ESTATUS_1000_THALF 			0x1000


/*
 *	Register 5 : Link Partner Ability
 */
#define BT_PHY_LPA_NPAGE 					0x8000
#define BT_PHY_LPA_LPACK 					0x4000
#define BT_PHY_LPA_RFAULT 					0x2000
#define BT_PHY_LPA_RESERVED_0 				0x1000	 	///< Technology Ability Field
#define BT_PHY_LPA_PAUSE_ASYM				0x0800
#define BT_PHY_LPA_PAUSE_CAP 				0x0400
#define BT_PHY_LPA_100BASE4 				0x0200
#define BT_PHY_LPA_1000XPAUSE_ASYM 			0x0100
#define BT_PHY_LPA_100FULL 					0x0100
#define BT_PHY_LPA_1000XPAUSE 				0x0080
#define BT_PHY_LPA_100HALF 					0x0080
#define BT_PHY_LPA_1000XHALF 				0x0040
#define BT_PHY_LPA_10FULL 					0x0040
#define BT_PHY_LPA_1000XFULL 				0x0020
#define BT_PHY_LPA_10HALF 					0x0020
#define BT_PHY_LPA_SLCT 					0x001F

/*
 *	Register 9 : Gigabit Control register
 */
#define BT_PHY_CTRL1000_1000FULL 			0x0200
#define BT_PHY_CTRL1000_1000HALF 			0x0100

/*
 *	Register 10 : 1000BASE-T Status Register
 */
#define BT_PHY_LPA_1000FULL 				0x0800
#define BT_PHY_LPA_1000HALF 				0x0400

#endif
