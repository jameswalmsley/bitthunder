/**
 *	Driver for Marvell PHYs in BitThunder.
 *
 **/
#include <bitthunder.h>

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
	{ 0x0000, 0x0009, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x7fff, 0x8000 },
	//{ 0x0000, 0x0000, 0xBFFF, 0x4000 },
};

static BT_HANDLE marvell_probe(BT_HANDLE hMII, struct bt_mii_bus *pbus, BT_u32 addr, BT_ERROR *pError) {

	BT_kPrint("marvell,phy: Detected initialising network phy.");

	const BT_DEV_IF_MII *pOps = BT_IF_MII_OPS(hMII);

	BT_u32 i;
	for(i = 0; i < sizeof(marvell_init)/sizeof(struct phy_config); i++) {
		pOps->pfnWrite(hMII, addr, 22, marvell_init[i].page);

		BT_u16 reg, mask, val;
		reg 	= marvell_init[i].reg;
		mask 	= marvell_init[i].mask;
		val 	= 0;

		if(mask) {
			val = pOps->pfnRead(hMII, addr, reg, pError);
			val &= mask;
		}

		val |= marvell_init[i].value;

		pOps->pfnWrite(hMII, addr, reg, val);
	}


	return NULL;
}

static struct bt_match_table_32 marvell_matchtable[] = {
	{ 0x01410dd0, 0xfffffff0 },
	{ }	// Must end with a NULL terminator
};

BT_INTEGRATED_DRIVER_DEF oMarvell = {
	.name  			= "marvell,phy",
	.eType 			= BT_DRIVER_MII,
	.pfnMIIProbe 	= marvell_probe,
	.pMatch32 		= marvell_matchtable,
};
