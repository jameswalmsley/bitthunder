
#include <string.h>

#include "net/bt_net.h"
#include "net/bt_lwip.h"
#include "collections/bt_fifo.h"
#include "../net/lwip/src/include/lwip/tcpip.h"
#include "interrupts/bt_tasklets.h"
#include "lwip/tcpip.h"


BT_DEF_MODULE_NAME			("NET IF Manager")
BT_DEF_MODULE_DESCRIPTION	("Network interface manager")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.co.at")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_LIST_HEAD(g_interfaces);
static BT_u32 n_interfaces=0;

static BT_HANDLE g_hMutex = NULL;
static BT_TASKLET sm_tasklet;

static BT_BOOL g_bDone = BT_FALSE;

static void net_event_handler(BT_NET_IF *pIF, BT_NET_IF_EVENT eEvent,
		BT_BOOL bInterruptContext) {
	BT_u32 ulWake;

	switch (eEvent) {

	case BT_NET_IF_RX_READY: {
		pIF->ulFlags |= DATA_READY;
		if (bInterruptContext) {
			BT_ReleaseMutexFromISR(g_hMutex, &ulWake);
		} else {
			BT_ReleaseMutex(g_hMutex);
		}
		break;
	}
	default:
		break;
	}

	//BT_TaskletSchedule(&sm_tasklet);
}

BT_ERROR BT_RegisterNetworkInterface(BT_HANDLE hIF) {

	BT_NETIF_PRIV *pNetIF = (BT_NETIF_PRIV *) BT_kMalloc(sizeof(BT_NETIF_PRIV));
	if (!pNetIF) {
		return BT_ERR_GENERIC;
	}

	memset(pNetIF, 0, sizeof(BT_NETIF_PRIV));

	pNetIF->base.hIF = hIF;
	pNetIF->base.pOps = hIF->h.pIf->oIfs.pDevIF->unConfigIfs.pEMacIF;
	pNetIF->base.name = pNetIF->netif.name;
#ifdef BT_CONFIG_NO_MODULE_STRINGS
	pNetIF->base.device_name = hIF->h.pIf->oInfo.szpModuleName;
#else
	pNetIF->base.device_name = hIF->h.pIf->oInfo.szpDescription;
#endif

	if (pNetIF->base.pOps->pfnEventSubscribe) {
		pNetIF->base.pOps->pfnEventSubscribe(hIF, net_event_handler,
				&pNetIF->base);
	}

	pNetIF->base.smFlags = NET_IF_ADDED;
	pNetIF->base.ulID = n_interfaces++;

	bt_list_add(&pNetIF->base.item, &g_interfaces);

	if (g_bDone) {
		BT_TaskletHighSchedule(&sm_tasklet);
	}

	BT_u8 mac[6];

	pNetIF->base.pOps->pfnGetMACAddr(hIF, mac, 6);
	BT_kPrint("Registered MAC with addr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_RegisterNetworkInterface);

static BT_NETIF_PRIV *find_netif(const BT_i8 *name) {
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_interfaces) {
		BT_NETIF_PRIV *netif = (BT_NETIF_PRIV *) pos;
		if(netif->base.smFlags & NET_IF_INITIALISED) {
			if(!strcmp(netif->base.name, name)) {
				return netif;
			}
		}
	}

	return NULL;
}


BT_u32 BT_GetTotalNetworkInterfaces() {
	BT_u32 i = 0;
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_interfaces) {
		i++;
	}
	return i;
}
BT_EXPORT_SYMBOL(BT_GetTotalNetworkInterfaces);

BT_NET_IF *BT_GetNetifByIndex(BT_u32 index) {

	BT_u32 i = 0;
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_interfaces) {
		if(i++ == index) {
			return (BT_NET_IF *) pos;
		}
	}

	return NULL;
}
BT_EXPORT_SYMBOL(BT_GetNetifByIndex);

BT_NET_IF *BT_GetNetif(const BT_i8 *name, BT_ERROR *pError) {
	BT_NETIF_PRIV *priv = find_netif(name);
	if(priv) {
		return &priv->base;
	}
	return NULL;
}
BT_EXPORT_SYMBOL(BT_GetNetif);

/**
 *	The PHY management layer calls this function when the PHY detects a change.
 *
 **/
BT_ERROR bt_netif_adjust_link(BT_NET_IF *netif) {

	BT_NETIF_PRIV *pIF = bt_container_of(netif, BT_NETIF_PRIV, base);

	if(!netif->phy) {
		return BT_ERR_GENERIC;
	}

	if(!netif->phy->link) {
		bt_lwip_netif_down(pIF);
	} else {
		bt_lwip_netif_up(pIF);
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_netif_adjust_link);

BT_NET_IF *BT_GetNetifFromHandle(BT_HANDLE hMac, BT_ERROR *pError) {
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_interfaces) {
		BT_NET_IF *netif = (BT_NET_IF *) pos;
		if(netif->hIF == hMac) {
			return netif;
		}
	}

	return NULL;
}
BT_EXPORT_SYMBOL(BT_GetNetifFromHandle);

BT_ERROR BT_NetifGetMacAddress(BT_NET_IF *interface, BT_u8 *hwaddr, BT_u32 ulLength) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return pIF->base.pOps->pfnGetMACAddr(pIF->base.hIF, hwaddr, ulLength);
}
BT_EXPORT_SYMBOL(BT_NetifGetMacAddress);

BT_ERROR BT_NetifSetMacAddress(BT_NET_IF *interface, BT_u8 *hwaddr, BT_u32 ulLength) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return pIF->base.pOps->pfnSetMACAddr(pIF->base.hIF, hwaddr, ulLength);
}
BT_EXPORT_SYMBOL(BT_NetifSetMacAddress);

BT_ERROR BT_NetifSetAddress(BT_NET_IF *interface, BT_IPADDRESS *ip, BT_IPADDRESS *netmask, BT_IPADDRESS *gw) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_set_addr(pIF, ip, netmask, gw);
}
BT_EXPORT_SYMBOL(BT_NetifSetAddress);

BT_ERROR BT_NetifGetAddress(BT_NET_IF *interface, BT_IPADDRESS *ip, BT_IPADDRESS *netmask, BT_IPADDRESS *gw) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_get_addr(pIF, ip, netmask, gw);
}
BT_EXPORT_SYMBOL(BT_NetifGetAddress);

BT_ERROR BT_StartNetif(BT_NET_IF *interface) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_up(pIF);
}
BT_EXPORT_SYMBOL(BT_StartNetif);

BT_ERROR BT_StopNetif(BT_NET_IF *interface) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_down(pIF);
}
BT_EXPORT_SYMBOL(BT_StopNetif);

BT_BOOL BT_NetifCompletedDHCP(BT_NET_IF *interface) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_dhcp_done(pIF);
}
BT_EXPORT_SYMBOL(BT_NetifCompletedDHCP);

BT_ERROR BT_NetifGetHostname(BT_NET_IF *interface, char *hostname) {
	BT_NETIF_PRIV *pIF = bt_container_of(interface, BT_NETIF_PRIV, base);
	return bt_lwip_netif_get_hostname(pIF, hostname);
}
BT_EXPORT_SYMBOL(BT_NetifGetHostname);

BT_ERROR BT_NetifGetLinkState(BT_NET_IF *interface, struct bt_phy_linkstate *linkstate) {
	if(!interface->phy) {
		return BT_ERR_GENERIC;
	}

	linkstate->link = interface->phy->link;
	linkstate->speed = interface->phy->speed;
	linkstate->duplex = interface->phy->duplex;
	linkstate->supported = interface->phy->supported;
	linkstate->advertising = interface->phy->advertising;
	linkstate->autoneg = interface->phy->autoneg;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_NetifGetLinkState);

BT_ERROR BT_NetifConfigureLink(BT_NET_IF *interface, struct bt_phy_config *config) {
	if(!interface->phy) {
		return BT_ERR_GENERIC;
	}

	interface->phy->advertising_mask = config->advertising_disable;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_NetifConfigureLink);

BT_ERROR BT_NetifRestartLink(BT_NET_IF *interface) {
	if(!interface->phy) {
		return BT_ERR_GENERIC;
	}

	interface->phy->eState = BT_PHY_RENEGOTIATE;
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_NetifRestartLink);

static void tcpip_init_done(void *arg) {
	BT_BOOL *bDone = (BT_BOOL*) arg;

	*bDone = BT_TRUE;
}

static BT_ERROR net_task(BT_HANDLE hThread, void *pParam) {
	BT_NETIF_PRIV *pIF;

	volatile BT_BOOL bDone = BT_FALSE;

	tcpip_init(tcpip_init_done, (BT_BOOL *) &bDone);

	while (!bDone) {
		BT_ThreadYield();
	}

	g_bDone = BT_TRUE;

	BT_TaskletHighSchedule(&sm_tasklet);

	while (1) {
		BT_PendMutex(g_hMutex, BT_INFINITE_TIMEOUT);

		struct bt_list_head *pos;
		bt_list_for_each(pos, &g_interfaces) {
			pIF = (BT_NETIF_PRIV *) pos;
			if (pIF->base.ulFlags & DATA_READY) {
				// Processes any packets waiting to be sent or received.
				bt_lwip_process(pIF);
				pIF->base.ulFlags &= ~DATA_READY;
				pIF->base.pOps->pfnSendEvent(pIF->base.hIF, BT_MAC_RECEIVED);
			}
		}
	}

	return BT_ERR_NONE;
}

static err_t lwip_init(struct netif *netif) {
	bt_lwip_netif_init((BT_NETIF_PRIV *) netif->state);

	return ERR_OK;
}

static void net_manager_sm(void *pParam) {

	BT_ERROR Error;

	struct ip_addr ip_addr;
	struct ip_addr net_mask;
	struct ip_addr gw_addr;

	ip_addr.addr = 0;
	net_mask.addr = 0;
	gw_addr.addr = 0;


	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_interfaces) {
		BT_NETIF_PRIV *pIF = (BT_NETIF_PRIV*) pos;
		if (pIF->base.smFlags & NET_IF_ADDED) {
			pIF->base.hTxFifo = BT_FifoCreate(20, sizeof(void *), &Error);

			pIF->netif.state = pIF;
			netifapi_netif_add(&pIF->netif, &ip_addr, &net_mask, &gw_addr, pIF,
					lwip_init, tcpip_input);

			netifapi_netif_set_default(&pIF->netif);
			//netifapi_dhcp_start(&pIF->netif);
		}

		if (pIF->base.smFlags & NET_IF_REMOVED) {
			BT_CloseHandle(pIF->base.hTxFifo);
		}
	}
}

BT_BOOL BT_isNetworkingReady() {
	return g_bDone;
}
BT_EXPORT_SYMBOL(BT_isNetworkingReady);

static BT_TASKLET sm_tasklet = { NULL, BT_TASKLET_IDLE, net_manager_sm, NULL };

static BT_ERROR bt_net_manager_init() {
	BT_ERROR Error = BT_ERR_NONE;

	g_hMutex = BT_CreateMutex(&Error);
	BT_PendMutex(g_hMutex, BT_INFINITE_TIMEOUT);

	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth = 256,
		.ulPriority = 0,
	};

	BT_CreateThread(net_task, &oThreadConfig, &Error);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	.pfnInit = bt_net_manager_init,
};
