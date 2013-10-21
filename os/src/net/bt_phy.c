/**
 *	Network PHY bus controller.
 *
 **/


#include <bitthunder.h>
#include <collections/bt_list.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

//static struct bt_list_head phy_devices;

BT_ERROR BT_RegisterMiiBus(BT_HANDLE hMII, struct bt_mii_bus *bus) {

	BT_ERROR Error = BT_ERR_NONE;
	const BT_DEV_IF_MII *pOps = BT_IF_MII_OPS(hMII);

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
						pDriver->pfnMIIProbe(hMII, bus, i, &Error);
						break;
					}
					pMatch++;
				}
			}
		}
	}

	return BT_ERR_NONE;
}
