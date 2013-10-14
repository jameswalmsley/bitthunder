#ifndef __BT_NET_H__
#define __BT_NET_H__

#include <bitthunder.h>

typedef struct _BT_NET_IF BT_NET_IF;

typedef enum _BT_NET_IF_EVENT {
	BT_NET_IF_RX_READY,
	BT_NET_IF_ADD_IF,
	BT_NET_IF_REMOVE_IF,
} BT_NET_IF_EVENT;

typedef enum _BT_MAC_EVENT {
	BT_MAC_RECEIVED,
	BT_MAC_TRANSMITTED,
} BT_MAC_EVENT;


typedef void (*BT_NET_IF_EVENTRECEIVER)(BT_NET_IF *pIF, BT_NET_IF_EVENT eEvent, BT_BOOL bInterruptContext);

typedef struct _BT_NET_IF_OPS {
	BT_u32 		ulCapabilities;			///< Primary Capability flags.

#define	BT_NET_IF_CAPABILITIES_ETHERNET				0x00000001
#define	BT_NET_IF_CAPABILITIES_100MBPS				0x00000002
#define	BT_NET_IF_CAPABILITIES_1000MBPS				0x00000004
#define	BT_NET_IF_CAPABILITIES_MDIX					0x00000002


	BT_ERROR 	(*pfnEventSubscribe)	(BT_HANDLE hIF, BT_NET_IF_EVENTRECEIVER pfnReceiver, BT_NET_IF *pIF);
	BT_ERROR	(*pfnInitialise)		(BT_HANDLE hIF);

	BT_ERROR	(*pfnGetMACAddr)		(BT_HANDLE hIF, BT_u8 *pAddr, BT_u32 ulLength);
	BT_ERROR	(*pfnSetMACAddr)		(BT_HANDLE hIF, BT_u8 *pAddr, BT_u32 ulLength);
	BT_u32 		(*pfnGetMTU)			(BT_HANDLE hIF, BT_ERROR *pError);

	BT_u32		(*pfnDataReady)			(BT_HANDLE hIF, BT_ERROR *pError);
	BT_ERROR	(*pfnRead)				(BT_HANDLE hIF, BT_u32 ulSize, void *pBuffer);
	BT_ERROR	(*pfnDropFrame)			(BT_HANDLE hIF, BT_u32 ulSize);
	BT_BOOL		(*pfnTxFifoReady)		(BT_HANDLE hIF, BT_ERROR *pError);
	BT_ERROR	(*pfnWrite)				(BT_HANDLE hIF, BT_u32 ulSize, void *pBuffer);
	BT_ERROR	(*pfnSendFrame)			(BT_HANDLE hIF);
	BT_ERROR	(*pfnSendEvent)			(BT_HANDLE hIF, BT_u32 ulEvent);
} BT_NET_IF_OPS;

typedef struct _BT_NET_IF {
	BT_LIST_ITEM			oItem;
	BT_HANDLE 				hIF;
	BT_HANDLE 				hTxFifo;
	BT_u32 					ulFlags;

#define	DATA_READY			0x00000001

	BT_u32 					ulIFFlags;

#define	BT_NETIF_FLAG_UP				NETIF_FLAG_UP
#define BT_NETIF_FLAG_BROADCAST			NETIF_FLAG_BROADCAST
#define BT_NETIF_FLAG_POINTTOPOINT		NETIF_FLAG_POINTTOPOINT
#define BT_NETIF_FLAG_DHCP				NETIF_FLAG_DHCP
#define BT_NETIF_FLAG_LINK_UP			NETIF_FLAG_LINK_UP
#define BT_NETIF_FLAG_ETHARP			NETIF_FLAG_ETHARP
#define BT_NETIF_FLAG_ETHERNET			NETIF_FLAG_ETHERNET
#define BT_NETIF_FLAG_IGMP				NETIF_FLAG_IGMP

	BT_u32 					smFlags;

#define	NET_IF_ADDED		0x00000001
#define	NET_IF_REMOVED		0x00000002

	const BT_NET_IF_OPS	   *pOps;

	BT_u32					ulID;
} BT_NET_IF;







BT_ERROR BT_RegisterNetworkInterface(BT_HANDLE hIF, const BT_NET_IF_OPS *pOps);

#endif
