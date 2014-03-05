/**
 *	Network PHY bus controller.
 *
 **/
#include <bitthunder.h>
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
static BT_HANDLE g_hPhyMutex = NULL;

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

	phy->advertising = phy->supported;

	return BT_ERR_NONE;
}

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
						BT_HANDLE hPHY = pDriver->pfnMIIProbe(hMII, bus, i, &Error);
						phy->hPHY = hPHY;
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

BT_ERROR BT_ConnectPHY(BT_HANDLE hMAC, BT_u32 ulAddress) {

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
							phy->eState = PHY_STARTING;	// Signal that this PHY can be brought online and the state machine can leave idle.
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

static BT_BOOL phy_sm(struct bt_phy_device *phy) {

	BT_BOOL clock_again = BT_FALSE;
	BT_ERROR Error = BT_ERR_NONE;

	switch(phy->eState) {

	case PHY_DOWN:					// NOP
		break;

	case PHY_STARTING:				// Get initial PHY state, and inform the MAC.
		phy->eState = PHY_CHANGELINK;
		BT_u16 ulCopperStatus = bt_phy_read(phy, 1, &Error);
		if(ulCopperStatus & BT_PHY_CSR_COPPER_LINK_STATUS) {
			BT_kPrint("PHY has an active link");
		}

		break;

	case PHY_UP:					// NOP : link_state -> state
		// Read the PHY status register, any change in the link?

		// YES ->
		phy->eState = PHY_CHANGELINK;
		clock_again = BT_TRUE;
		break;

	case PHY_CHANGELINK: {
		// Signal to the MAC that speed or link state has changed on the PHY.
		// This allows the MAC to change its RX/TXD clocks, or stop processing data when PHY is down.

		const BT_DEV_IF_EMAC *mac_ops = BT_IF_EMAC_OPS(phy->active_mac);
		mac_ops->adjust_link(phy->active_mac, phy);

		return BT_TRUE;
		break;
	}

	case PHY_NOLINK:				// NOP : link_state -> state
		break;

	default:
		break;
	}

	return clock_again;				// Returning FALSE signals that the SM doesn't require clocking until the next poll.
}

static BT_ERROR phy_task(BT_HANDLE hThread, void *pParam) {

	while(1) {
		BT_kPrint("Polling the PHYs");
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

BT_ERROR phy_module_init() {

	BT_ERROR Error = BT_ERR_NONE;
	g_hPhyMutex = BT_CreateMutex(&Error);

	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth = 256,
		.ulPriority = 0,
	};

	BT_CreateThread(phy_task, &oThreadConfig, &Error);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oPhyModuleEntry = {
	BT_MODULE_NAME,
	.pfnInit = phy_module_init,
};
