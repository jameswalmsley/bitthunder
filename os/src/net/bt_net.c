
#include <string.h>

#include "net/bt_net.h"
#include "net/bt_lwip.h"
#include "collections/bt_fifo.h"
#include "../net/lwip/src/include/lwip/tcpip.h"
#include "interrupts/bt_tasklets.h"
#include "lwip/tcpip.h"

BT_DEF_MODULE_NAME("NET IF Manager")
BT_DEF_MODULE_DESCRIPTION("Network interface manager")
BT_DEF_MODULE_AUTHOR("Robert Steinbauer")
BT_DEF_MODULE_EMAIL("rsteinbauer@riegl.co.at")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_LIST g_oInterfaces = { NULL };

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

	if (pNetIF->base.pOps->pfnEventSubscribe) {
		pNetIF->base.pOps->pfnEventSubscribe(hIF, net_event_handler,
				&pNetIF->base);
	}

	pNetIF->base.smFlags = NET_IF_ADDED;

	BT_ListAddItem(&g_oInterfaces, &pNetIF->base.oItem);

	if (g_bDone) {
		BT_TaskletHighSchedule(&sm_tasklet);
	}
	BT_u8 mac[6];

	pNetIF->base.pOps->pfnGetMACAddr(hIF, mac, 6);
	BT_kPrint("Registered MAC with addr: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return BT_ERR_NONE;
}

static void tcpip_init_done(void *arg) {
	BT_BOOL *bDone = (BT_BOOL*) arg;

	*bDone = BT_TRUE;
}

static void net_task(void *pParam) {
	BT_NETIF_PRIV *pIF;

	BT_BOOL bDone = BT_FALSE;

	tcpip_init(tcpip_init_done, &bDone);

	while (!bDone) {
		BT_ThreadYield();
	}

	g_bDone = BT_TRUE;

	BT_TaskletHighSchedule(&sm_tasklet);

	while (1) {
		BT_PendMutex(g_hMutex, 0);

		pIF = (BT_NETIF_PRIV*) BT_ListGetHead(&g_oInterfaces);
		while (pIF) {

			if (pIF->base.ulFlags & DATA_READY) {
				// Processes any packets waiting to be sent or received.

				bt_lwip_process(pIF);

				pIF->base.ulFlags &= ~DATA_READY;

				pIF->base.pOps->pfnSendEvent(pIF->base.hIF, BT_MAC_RECEIVED);
			}

			pIF = (BT_NETIF_PRIV*) BT_ListGetNext(&pIF->base.oItem);
		}
	}

	BT_kTaskDelete(NULL );
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

	BT_NETIF_PRIV *pIF = (BT_NETIF_PRIV*) BT_ListGetHead(&g_oInterfaces);
	while (pIF) {
		if (pIF->base.smFlags & NET_IF_ADDED) {

			bt_lwip_netif_init(pIF);

			pIF->base.hTxFifo = BT_FifoCreate(20, sizeof(void *), 0, &Error);

			pIF->netif.state = pIF;
			netifapi_netif_add(&pIF->netif, &ip_addr, &net_mask, &gw_addr, pIF,
					lwip_init, tcpip_input);

			netifapi_netif_set_default(&pIF->netif);
			netifapi_dhcp_start(&pIF->netif);
		}
		if (pIF->base.smFlags & NET_IF_REMOVED) {
			BT_CloseHandle(pIF->base.hTxFifo);
		}

		pIF = (BT_NETIF_PRIV*) BT_ListGetNext(&pIF->base.oItem);
	}

}

static BT_TASKLET sm_tasklet = { NULL, BT_TASKLET_IDLE, net_manager_sm, NULL };

static BT_ERROR bt_net_manager_init() {
	BT_ERROR Error = BT_ERR_NONE;

	BT_ListInit(&g_oInterfaces);

	g_hMutex = BT_CreateMutex(&Error);
	BT_PendMutex(g_hMutex, 0);

	BT_THREAD_CONFIG oThreadConfig = {
		.ulStackDepth = 256,
		.ulPriority = 0,
	};

	BT_kTaskCreate(net_task, "netif", &oThreadConfig, &Error);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_0_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_net_manager_init,
};
