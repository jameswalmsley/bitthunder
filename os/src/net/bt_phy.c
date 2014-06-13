/**
 *	Network PHY bus controller.
 *
 **/
#include <bitthunder.h>
#include <of/bt_of.h>
#include <collections/bt_list.h>
#include <string.h>

BT_DEF_MODULE_NAME			("NET PHY Manager");
BT_DEF_MODULE_DESCRIPTION	("Network PHY control and interface manager");
BT_DEF_MODULE_AUTHOR		("James Walmsley");
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk");

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_LIST_HEAD(g_mii_busses);

#define MII_LOCK(x)
#define MII_UNLOCK(x)

#define PHY_LOCK(x)		MII_LOCK(x->mii_bus)
#define PHY_UNLOCK(x)	MII_UNLOCK(x->mii_bus)

static BT_ERROR bt_phy_generic_update_link(struct bt_phy_device *phy) {
	BT_ERROR Error = BT_ERR_NONE;

	BT_u16 status = bt_phy_read(phy, BT_PHY_MII_BMSR, &Error);
	status = bt_phy_read(phy, BT_PHY_MII_BMSR, &Error);

	if(status & BT_PHY_BMSR_LINK_STATUS) {
		phy->link = 1;
	} else {
		phy->link = 0;
	}

	return BT_ERR_NONE;
}


BT_ERROR bt_phy_generic_read_status(struct bt_phy_device *phy) {
	bt_phy_generic_update_link(phy);

	BT_ERROR Error = BT_ERR_NONE;

	BT_u16 lpagb = 0;
	BT_u16 lpa;
	BT_u16 adv;

	if(phy->autoneg) {
		if(phy->supported & (BT_PHY_SUPPORTED_1000baseT_Half | BT_PHY_SUPPORTED_1000baseT_Full)) {
			lpagb = bt_phy_read(phy, BT_PHY_MII_STAT1000, &Error);
			adv = bt_phy_read(phy, BT_PHY_MII_CTRL1000, &Error);
			lpagb &= adv << 2;
		}

		lpa = bt_phy_read(phy, BT_PHY_MII_LPA, &Error);
		adv = bt_phy_read(phy, BT_PHY_MII_ADVERTISE, &Error);

		lpa &= adv;

		phy->speed = 10;
		phy->duplex = BT_PHY_DUPLEX_HALF;
		phy->pause = phy->asym_pause = 0;

		if(lpagb & (BT_PHY_LPA_1000FULL | BT_PHY_LPA_1000HALF)) {
			phy->speed = 1000;
			if(lpagb & BT_PHY_LPA_1000FULL)
				phy->duplex = BT_PHY_DUPLEX_FULL;
		} else if(lpa & (BT_PHY_LPA_100FULL | BT_PHY_LPA_100HALF)) {
			phy->speed = 100;
			if(lpa & BT_PHY_LPA_100FULL)
				phy->duplex = BT_PHY_DUPLEX_FULL;
		} else {
			if(lpa & (BT_PHY_LPA_10FULL)) {
				phy->duplex = BT_PHY_DUPLEX_FULL;
			}
		}

		if(phy->duplex == BT_PHY_DUPLEX_FULL) {
			phy->pause = lpa & BT_PHY_LPA_PAUSE_CAP ? 1 : 0;
			phy->asym_pause = lpa & BT_PHY_LPA_PAUSE_ASYM ? 1 : 0;
		}
	} else {
		BT_u16 bmcr = bt_phy_read(phy, BT_PHY_MII_BMCR, &Error);

		if(bmcr & BT_PHY_BMCR_COPPER_DUPLEX_MODE) {
			phy->duplex = BT_PHY_DUPLEX_FULL;
		} else {
			phy->duplex = BT_PHY_DUPLEX_HALF;
		}

		if(bmcr & BT_PHY_BMCR_SPEED_SELECT_MSB) {
			phy->speed = 1000;
		} else if(bmcr & BT_PHY_BMCR_SPEED_SELECT_LSB) {
			phy->speed = 100;
		} else {
			phy->speed = 10;
		}
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_phy_generic_read_status);

BT_ERROR bt_phy_generic_init(struct bt_phy_device *phy) {

	BT_ERROR Error = BT_ERR_NONE;

	// Assume support for all features :S
	phy->supported = (BT_PHY_SUPPORTED_TP | BT_PHY_SUPPORTED_MII | BT_PHY_SUPPORTED_AUI
					   | BT_PHY_SUPPORTED_FIBRE | BT_PHY_SUPPORTED_BNC);

	BT_u16 val = bt_phy_read(phy, BT_PHY_MII_BMSR, &Error);

	if(val & BT_PHY_BMSR_AUTONEG_ABILITY) {
		phy->supported |= BT_PHY_SUPPORTED_Autoneg;
	}

	if(val & BT_PHY_BMSR_100BASE_X_FULL_DUPLEX)
		phy->supported |= BT_PHY_SUPPORTED_100baseT_Full;

	if(val & BT_PHY_BMSR_100BASE_X_HALF_DUPLEX)
		phy->supported |= BT_PHY_SUPPORTED_100baseT_Half;

	if(val & BT_PHY_BMSR_10MBPS_FULL_DUPLEX)
		phy->supported |= BT_PHY_SUPPORTED_10baseT_Full;

	if(val & BT_PHY_BMSR_10MBPS_HALF_DUPLEX)
		phy->supported |= BT_PHY_SUPPORTED_10baseT_Half;

	if(val & BT_PHY_BMSR_EXTENDED_STATUS) {
		val = bt_phy_read(phy, BT_PHY_MII_ESTATUS, &Error);

		if(val & BT_PHY_ESTATUS_1000_TFULL)
			phy->supported |= BT_PHY_SUPPORTED_1000baseT_Full;

		if(val & BT_PHY_ESTATUS_1000_THALF)
			phy->supported |= BT_PHY_SUPPORTED_1000baseT_Half;
	}

	phy->advertising = phy->supported & ~phy->advertising_mask;

	val = bt_phy_read(phy, BT_PHY_MII_CTRL1000, &Error);
	// Update the advertising flags on the PHY.
	if(phy->advertising_mask & BT_PHY_SUPPORTED_1000baseT_Half) {
		val &= ~BT_PHY_CTRL1000_1000HALF;
	}

	if(phy->advertising_mask & BT_PHY_SUPPORTED_1000baseT_Full) {
		val &= ~BT_PHY_CTRL1000_1000FULL;
	}

	bt_phy_write(phy, BT_PHY_MII_CTRL1000, val);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_phy_generic_init);

static BT_ERROR phy_init(struct bt_phy_device *phy) {
	const BT_DEV_IF_PHY *phy_ops = BT_IF_PHY_OPS(phy->hPHY);
	if(phy_ops->pfnConfigInit) {
		return phy_ops->pfnConfigInit(phy);
	}

	return bt_phy_generic_init(phy);
}

static BT_ERROR phy_read_status(struct bt_phy_device *phy) {
	const BT_DEV_IF_PHY *phy_ops = BT_IF_PHY_OPS(phy->hPHY);
	if(phy_ops->pfnReadStatus) {
		return phy_ops->pfnReadStatus(phy);
	}

	return bt_phy_generic_read_status(phy);
}

BT_ERROR BT_RegisterMiiBus(BT_HANDLE hMII, struct bt_mii_bus *bus) {

	BT_ERROR Error = BT_ERR_NONE;
	const BT_DEV_IF_MII *pOps = BT_IF_MII_OPS(hMII);

	BT_LIST_INIT_HEAD(&bus->phys);				// Initialise the list of PHYs on this bus.
	bus->hMII = hMII;
	bus->mutex = BT_kMutexCreate();

#ifdef BT_CONFIG_OF
	/*
	 *	bus->pDevice should be now pointing to a parental BT_DEVICE structure.
	 *	Usually this is a MAC peripheral which contains MDIO logic.
	 */
	struct bt_device_node *mdio_parent = bt_of_integrated_get_node(bus->pDevice);	// Cast to the device tree.
	if(!mdio_parent) {
		goto no_node;
	}

	struct bt_device_node *mdio_node = bt_of_mdio_get_node(mdio_parent);			// Get an MDIO bus node.
	if(!mdio_node) {
		goto no_node;
	}

	bt_of_mdio_populate_device(mdio_node);	// Populate the described PHY devices.

	BT_DEVICE *pDevice = &mdio_node->dev;
	bus->pDevice = pDevice;

no_node:
#endif

	bt_list_add(&bus->item, &g_mii_busses);

	BT_u32 i;
	for(i = 0; i < 31; i++) {
		BT_u32 reg2 = pOps->pfnRead(hMII, i, 2, &Error);
		BT_u32 reg3 = pOps->pfnRead(hMII, i, 3, &Error);

		BT_u32 id = reg2 << 16 | reg3;

		if(reg2 != 0xFFFF) {
			BT_kPrint("PHY Detected on: %s : PHY %d: reg2 %08x, reg3 %08x", bus->name, i, reg2, reg3);
			BT_u32 drivers = BT_GetTotalIntegratedDriversByType(BT_DRIVER_MII);
			BT_u32 y;
			for(y = 0; y < drivers; y++) {
				BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByType(BT_DRIVER_MII, y);
				if(!pDriver || !pDriver->pMatch32) {
					continue;
				}

				struct bt_match_table_32 *pMatch = pDriver->pMatch32;
				while(pMatch) {
					if((id & pMatch->id_mask) == pMatch->id) {
						struct bt_phy_device *phy = BT_kMalloc(sizeof(*phy));
						memset(phy, 0, sizeof(*phy));
						BT_kPrint("PHY driver detected as: %s", pDriver->name);

						/*
						 *	If this device is described in the device tree, the we can use the BT_DEVICE structure provided,
						 * 	Otherwise, we must create a BT_DEVICE for the phy.
						 */

						struct bt_device_node *phy_device = NULL;

						struct bt_device_node *dev = bt_of_integrated_get_node(bus->pDevice);	// Cast out to device tree if available.
						if(dev) {
							struct bt_list_head *pos;
							bt_list_for_each(pos, &dev->children) {
								struct bt_device_node *phy_node = bt_container_of(pos, struct bt_device_node, item);
								const BT_be32 *phy_address = bt_of_get_address(phy_node, 0, NULL, NULL);
								if(bt_be32_to_cpu(*phy_address) == i) {
									// Sucessfully matched a node in the DT.
									phy_device = phy_node;
								}
							}

						}

						const BT_DEVICE *phy_legacy_device = phy_device ? &phy_device->dev : NULL;

						phy->mii_bus	= bus;
						phy->pDevice 	= phy_legacy_device;
						phy->phy_id 	= i;

						BT_HANDLE hPHY = pDriver->pfnMIIProbe(phy, &Error);
						phy->hPHY = hPHY;
						phy->mii_bus = bus;
						phy->autoneg = 1;
						phy->speed = 0;

						phy_init(phy);

						bt_list_add(&phy->item, &bus->phys);
						break;
					}
					pMatch++;
				}
			}
		}
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_RegisterMiiBus);

BT_ERROR BT_ConnectPHY(BT_HANDLE hMAC, BT_u32 ulAddress) {

	BT_ERROR Error = BT_ERR_NONE;

	struct bt_list_head *bus_pos;
	bt_list_for_each(bus_pos, &g_mii_busses) {
		struct bt_mii_bus *bus = bt_container_of(bus_pos, struct bt_mii_bus, item);
		if(hMAC == bus->hMAC) {	// Mac wants to connect with a PHY on this bus.

			struct bt_list_head *pos;
			bt_list_for_each(pos, &bus->phys) {
				struct bt_phy_device *phy = bt_container_of(pos, struct bt_phy_device, item);

				PHY_LOCK(phy);
				{
					if(phy->phy_id == ulAddress) {
						if(!phy->active_mac) {
							phy->active_mac = hMAC;
							phy->eState = BT_PHY_STARTING;	// Signal that this PHY can be brought online and the state machine can leave idle.
							BT_NET_IF *netif = BT_GetNetifFromHandle(phy->active_mac, &Error);
							netif->phy = phy;
							return BT_ERR_NONE;
						}
					}
				}
				PHY_UNLOCK(phy);
			}
		}
	}

	return BT_ERR_GENERIC;	// PHY not detected on specified address.
}
BT_EXPORT_SYMBOL(BT_ConnectPHY);

static BT_BOOL phy_sm(struct bt_phy_device *phy) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_BOOL clock_again = BT_FALSE;

	switch(phy->eState) {

	case BT_PHY_DOWN:					// NOP
		phy_read_status(phy);
		if(phy->link) {
			phy->eState = BT_PHY_CHANGELINK;
			clock_again = BT_TRUE;
		}
		break;

	case BT_PHY_STARTING:				// Initialise the PHY and move into PHY_DOWN.
	{
		phy_init(phy);
		phy->eState = BT_PHY_RENEGOTIATE;
		clock_again = BT_TRUE;
		break;
	}

	case BT_PHY_UP:						// NOP : link_state -> state
	{
		// Read the PHY status register, any change in the link?
		phy_read_status(phy);

		if(!phy->link) {
			phy->eState = BT_PHY_CHANGELINK;
			clock_again = BT_TRUE;
		}

		break;
	}

	case BT_PHY_RENEGOTIATE:
	{
		phy_init(phy);

		BT_u16 bmcr = bt_phy_read(phy, BT_PHY_MII_BMCR, &Error);
		bmcr |= BT_PHY_BMCR_RESET;
		bt_phy_write(phy, BT_PHY_MII_BMCR, bmcr);

		phy->eState = BT_PHY_DOWN;
		clock_again = BT_TRUE;
		break;
	}

	case BT_PHY_CHANGELINK:
	{
		// Signal to the MAC that speed or link state has changed on the PHY.
		// This allows the MAC to change its RX/TXD clocks, or stop processing data when PHY is down.

		const BT_DEV_IF_EMAC *mac_ops = BT_IF_EMAC_OPS(phy->active_mac);
		mac_ops->adjust_link(phy->active_mac, phy);

		if(phy->link) {
			phy->eState = BT_PHY_UP;
		} else {
			phy->eState = BT_PHY_DOWN;
		}

		// Notify the netif and stack of link state change
		BT_NET_IF *netif = BT_GetNetifFromHandle(phy->active_mac, &Error);
		bt_netif_adjust_link(netif);
		break;
	}

	case BT_PHY_NOLINK:				// NOP : link_state -> state
	{
		break;
	}

	default:
		break;
	}

	return clock_again;				// Returning FALSE signals that the SM doesn't require clocking until the next poll.
}

static BT_ERROR phy_task(BT_HANDLE hThread, void *pParam) {

	while(1) {
		struct bt_list_head *pos_bus;
		bt_list_for_each(pos_bus, &g_mii_busses) {	// Iterate through the mii busses
			struct bt_mii_bus *bus = bt_container_of(pos_bus, struct bt_mii_bus, item);

			struct bt_list_head *pos;
			bt_list_for_each(pos, &bus->phys) {		// Iterate through the phys on each bus.
				struct bt_phy_device *phy = bt_container_of(pos, struct bt_phy_device, item);
				BT_BOOL clock_again = BT_TRUE;
				while(clock_again) {
					clock_again = phy_sm(phy);		// Run the PHY state machine on this PHY, keep clocking (calling) it until its stabilised.
				}
			}
		}

		BT_ThreadSleep(1000);	// Poll every PHY each second.
	}

	return BT_ERR_NONE;
}

BT_u16 bt_phy_read(struct bt_phy_device *phy, BT_u32 regnum, BT_ERROR *pError) {
	const BT_DEV_IF_MII *mii_ops = BT_IF_MII_OPS(phy->mii_bus->hMII);
	BT_u16 reg = 0;

	PHY_LOCK(phy);
	{
		reg = mii_ops->pfnRead(phy->mii_bus->hMII, phy->phy_id, regnum, pError);
	}
	PHY_UNLOCK(phy);

	return reg;
}
BT_EXPORT_SYMBOL(bt_phy_read);

BT_ERROR bt_phy_write(struct bt_phy_device *phy, BT_u32 regnum, BT_u16 val) {
	const BT_DEV_IF_MII *mii_ops = BT_IF_MII_OPS(phy->mii_bus->hMII);

	BT_ERROR Error = BT_ERR_NONE;

	PHY_LOCK(phy);
	{
		Error = mii_ops->pfnWrite(phy->mii_bus->hMII, phy->phy_id, regnum, val);
	}
	PHY_UNLOCK(phy);

	return Error;
}
BT_EXPORT_SYMBOL(bt_phy_write);

static BT_ERROR phy_module_init() {

	BT_ERROR Error = BT_ERR_NONE;

	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth = 512,
		.ulPriority = 0,
	};

	BT_CreateThread(phy_task, &oThreadConfig, &Error);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oPhyModuleEntry = {
	BT_MODULE_NAME,
	.pfnInit = phy_module_init,
};
