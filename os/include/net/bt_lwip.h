#ifndef __BT_LWIP_H__
#define __BT_LWIP_H__

#include <bitthunder.h>
#include "../src/net/lwip/src/include/lwip/netif.h"

typedef struct _BT_NETIF_PRIV {
	BT_NET_IF base;
	struct netif netif;
} BT_NETIF_PRIV;


BT_ERROR	bt_lwip_netif_init	(BT_NETIF_PRIV *pIF);
void		bt_lwip_process		(BT_NETIF_PRIV *pIF);


#endif
