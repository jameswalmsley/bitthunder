#ifndef __BT_NET_H__
#define __BT_NET_H__

#include <bitthunder.h>
#include <collections/bt_list.h>
#include "bt_phy.h"

typedef struct _BT_NET_IF {
	struct bt_list_head		item;
	const BT_i8 		   *name;
	const BT_i8			   *device_name;	///< Description string from the device driver.
	BT_HANDLE 				hIF;
	struct bt_phy_device   *phy;
	BT_HANDLE 				hTxFifo;
	BT_u32 					ulFlags;

#define	DATA_READY			0x00000001

	BT_u32 					ulIFFlags;
#define	BT_NETIF_FLAG_UP				0x00000001
#define BT_NETIF_FLAG_BROADCAST			0x00000002
#define BT_NETIF_FLAG_POINTTOPOINT		0x00000004
#define BT_NETIF_FLAG_DHCP				0x00000008
#define BT_NETIF_FLAG_LINK_UP			0x00000010
#define BT_NETIF_FLAG_ETHARP			0x00000020
#define BT_NETIF_FLAG_ETHERNET			0x00000040
#define BT_NETIF_FLAG_IGMP				0x00000080

	BT_u32 					smFlags;
#define	NET_IF_ADDED		0x00000001
#define	NET_IF_REMOVED		0x00000002
#define NET_IF_INITIALISED	0x00000004

	const BT_DEV_IF_EMAC   *pOps;
	BT_u32					ulID;
} BT_NET_IF;



BT_ERROR BT_RegisterNetworkInterface(BT_HANDLE hIF);
BT_BOOL BT_isNetworkingReady();

BT_NET_IF *BT_GetNetif(const BT_i8 *name, BT_ERROR *pError);
BT_NET_IF *BT_GetNetifFromHandle(BT_HANDLE hMac, BT_ERROR *pError);

BT_ERROR BT_NetifGetMacAddress(BT_NET_IF *interface, BT_u8 *hwaddr, BT_u32 ulLength);
BT_ERROR BT_NetifSetMacAddress(BT_NET_IF *interface, BT_u8 *hwaddr, BT_u32 ulLength);

BT_ERROR BT_NetifSetAddress(BT_NET_IF *interface, BT_IPADDRESS *ip, BT_IPADDRESS *netmask, BT_IPADDRESS *gw);
BT_ERROR BT_NetifGetAddress(BT_NET_IF *interface, BT_IPADDRESS *ip, BT_IPADDRESS *netmask, BT_IPADDRESS *gw);

/**
 *	@public
 *	@brief 	Allows configuration of PHY/Link modes.
 *
 *	@param 	[IN] interface	Network interface to be configured.
 *	@param	[IN] config		Structure containing the configuration flags to be applied.
 *
 *	@return	BT_ERR_NONE		On sucessful application of the configuration.
 **/
BT_ERROR BT_NetifConfigureLink(BT_NET_IF *interface, struct bt_phy_config *config);

/**
 *	@brief	Resets the PHY/Link after a reconfiguration.
 *
 **/
BT_ERROR BT_NetifRestartLink(BT_NET_IF *interface);

BT_ERROR BT_NetifGetLinkState(BT_NET_IF *interface, struct bt_phy_linkstate *linkstate);

BT_ERROR BT_StartNetif(BT_NET_IF *interface);
BT_ERROR BT_StopNetif(BT_NET_IF *interface);

BT_ERROR BT_NetifGetHostname(BT_NET_IF *interface, char *hostname);

BT_ERROR bt_netif_adjust_link(BT_NET_IF *netif);

#endif
