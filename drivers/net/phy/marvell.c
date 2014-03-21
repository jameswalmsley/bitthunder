/**
 *	Driver for Marvell PHYs in BitThunder.
 *
 **/
#include <bitthunder.h>
#include <of/bt_of.h>

BT_DEF_MODULE_NAME					("marvell-phy")
BT_DEF_MODULE_DESCRIPTION			("Marvell PHY driver for BitThunder")
BT_DEF_MODULE_AUTHOR				("James Walmsley")
BT_DEF_MODULE_EMAIL					("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

struct phy_config {
	BT_u16 page;
	BT_u16 reg;
	BT_u16 mask;
	BT_u16 value;
};

struct phy_config marvell_init[] = {
	{ 0x0003, 0x0010, 0xff00, 0x001e },
	{ 0x0003, 0x0010, 0xfff0, 0x000a },
	{ 0x0000, 0x0000, 0x0000, 0x1000 },
	{ 0x0000, 0x0004, 0x0000, 0x01E1 },
	{ 0x0000, 0x0009, 0x0000, 0x0300 },
	{ 0x0000, 0x0000, 0x7fff, 0x8000 },
	//{ 0x0000, 0x0000, 0xBFFF, 0x4000 },
};

static const BT_IF_HANDLE oMarvellHandleInterface;

static BT_HANDLE marvell_probe(struct bt_phy_device *phy, BT_ERROR *pError) {


	BT_ERROR Error = BT_ERR_NONE;

	/*
	 *	Note this is a device on a BUS which supports discovery, its possible that phy->pDevice is NULL!
	 */

	BT_HANDLE hPHY = BT_CreateHandle(&oMarvellHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hPHY) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	return hPHY;

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

static BT_ERROR marvell_phy_init(struct bt_phy_device *phy) {

	BT_ERROR Error = BT_ERR_NONE;

	struct bt_device_node *phy_node = bt_of_integrated_get_node(phy->pDevice);
	if(phy_node) {
		BT_kPrint("Found a phy_node in the device tree");
	}

	// Apply any init reg values from the device tree if available.
	BT_u32 i;
	for(i = 0; i < sizeof(marvell_init)/sizeof(struct phy_config); i++) {

		bt_phy_write(phy, 22, marvell_init[i].page);

		BT_u16 reg, mask, val;
		reg 	= marvell_init[i].reg;
		mask 	= marvell_init[i].mask;
		val 	= 0;

		if(mask) {
			val = bt_phy_read(phy, reg, &Error);
			val &= mask;
		}

		val |= marvell_init[i].value;

		bt_phy_write(phy, reg, val);
	}

	return bt_phy_generic_init(phy);
}

static struct bt_match_table_32 marvell_matchtable[] = {
	{ 0x01410dd0, 0xfffffff0 },
	{ }	// Must end with a NULL terminator
};


static const BT_DEV_IF_PHY phy_ops = {
	.pfnConfigInit = marvell_phy_init,				// Leaving the methods as NULL falls back to generic implementation.
	//.pfnReadStatus = bt_phy_generic_read_status,
	//.pfnConfigAneg = marvell_aneg,
};

static const BT_IF_DEVICE oMarvellDeviceIF = {
	.eConfigType = BT_DEV_IF_T_PHY,
	.unConfigIfs = {
		.pPhyIF = &phy_ops,
	},
};

static const BT_IF_HANDLE oMarvellHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oMarvellDeviceIF,
	},
	.eType = BT_HANDLE_T_DEVICE,
};

BT_INTEGRATED_DRIVER_DEF oMarvell = {
	.name  			= "marvell,phy",
	.eType 			= BT_DRIVER_MII,
	.pfnMIIProbe 	= marvell_probe,
	.pMatch32 		= marvell_matchtable,
};
