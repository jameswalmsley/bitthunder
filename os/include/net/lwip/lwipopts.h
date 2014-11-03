#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <bt_config.h>

#define	TCPIP_THREAD_STACKSIZE			256

/* Prevent having to link sys_arch.c (we don't test the API layers in unit tests) */
#define NO_SYS                          0
#define LWIP_NETCONN                    1
#define	LWIP_COMPAT_SOCKETS				0
#define LWIP_SOCKET                     1
#define LWIP_PROVIDE_ERRNO				1
#define	SYS_LIGHTWEIGHT_PROT			1
#define LWIP_NETIF_API                  1

#define LWIP_TCP						BT_CONFIG_USE_TCP
#define LWIP_UDP						BT_CONFIG_USE_UDP
#define LWIP_DHCP						BT_CONFIG_USE_DHCP
#define LWIP_IGMP						BT_CONFIG_USE_IGMP
#define LWIP_NETIF_HOSTNAME				BT_CONFIG_USE_DHCP
#define LWIP_DNS						1
#define DNS_TABLE_SIZE                  2
#define DNS_MAX_NAME_LENGTH             64


#define LWIP_SO_RCVTIMEO				1


#define TCP_MSS							BT_CONFIG_NET_LWIP_TCP_MSS
#define MEM_SIZE						BT_CONFIG_NET_LWIP_MEM_SIZE
#define MEM_ALIGNMENT                   4
#define TCP_SND_BUF                     (3 * TCP_MSS)
#ifdef BT_CONFIG_MACH_ETHERNET_BUFFER
#define TCP_WND                         (BT_CONFIG_MACH_ETHERNET_BUFFER-256)
#else
#define TCP_WND                         (2 * TCP_MSS)
#endif


//*****************************************************************************
// ---------- Pbuf options ----------
//*****************************************************************************
#ifdef BT_CONFIG_MACH_ETH_PAD_SIZE
#define ETH_PAD_SIZE                    BT_CONFIG_MACH_ETH_PAD_SIZE		// to ensure 32 bit alignment
#else
#define ETH_PAD_SIZE                    0		// to ensure 32 bit alignment
#endif


/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES   1


//#define	LWIP_COMPAT_MUTEX				1

#define TCPIP_MBOX_SIZE                 10
#define DEFAULT_RAW_RECVMBOX_SIZE       10
#define DEFAULT_UDP_RECVMBOX_SIZE       10
#define DEFAULT_TCP_RECVMBOX_SIZE       10
#define DEFAULT_ACCEPTMBOX_SIZE         10

#ifdef BT_CONFIG_NET_LWIP_GEN_CHECKSUM
	#define CHECKSUM_GEN_IP					0
	#define CHECKSUM_GEN_UDP				0
	#define CHECKSUM_GEN_TCP 				0
#else
	#define CHECKSUM_GEN_IP					1
	#define CHECKSUM_GEN_UDP				1
	#define CHECKSUM_GEN_TCP 				1
#endif

#define MEMP_NUM_PBUF					BT_CONFIG_NET_LWIP_MEMP_NUM_PBUF
#define MEMP_NUM_RAW_PCB				BT_CONFIG_NET_LWIP_MEMP_NUM_RAW_PCB
#define MEMP_NUM_UDP_PCB 				BT_CONFIG_NET_LWIP_MEMP_NUM_UDP_PCB
#define MEMP_NUM_TCP_PCB				BT_CONFIG_NET_LWIP_MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB_LISTEN 		BT_CONFIG_NET_LWIP_MEMP_NUM_TCP_PCB_LISTEN
#define MEMP_NUM_TCP_SEG 				BT_CONFIG_NET_LWIP_MEMP_NUM_TCP_SEG
#define MEMP_NUM_REASSDATA				BT_CONFIG_NET_LWIP_MEMP_NUM_REASSDATA
#define MEMP_NUM_FRAG_PBUF 				BT_CONFIG_NET_LWIP_MEMP_NUM_FRAG_PBUF
#define MEMP_NUM_ARP_QUEUE 				BT_CONFIG_NET_LWIP_MEMP_NUM_ARP_QUEUE
#define MEMP_NUM_IGMP_GROUP 			BT_CONFIG_NET_LWIP_MEMP_NUM_IGMP_GROUP
#define MEMP_NUM_NETBUF					BT_CONFIG_NET_LWIP_MEMP_NUM_NETBUF
#define MEMP_NUM_NETCONN				BT_CONFIG_NET_LWIP_MEMP_NUM_NETCONN
#define MEMP_NUM_TCPIP_MSG_API 			BT_CONFIG_NET_LWIP_MEMP_NUM_TCPIP_MSG_API
#define MEMP_NUM_TCPIP_MSG_INPKT 		BT_CONFIG_NET_LWIP_MEMP_NUM_TCPIP_MSG_INPKT
#define MEMP_NUM_SNMP_NODE 				BT_CONFIG_NET_LWIP_MEMP_NUM_SNMP_NODE
#define MEMP_NUM_SNMP_ROOTNODE			BT_CONFIG_NET_LWIP_MEMP_NUM_SNMP_ROOTNODE
#define MEMP_NUM_SNMP_VARBIND 			BT_CONFIG_NET_LWIP_MEMP_NUM_SNMP_VARBIND
#define PBUF_POOL_SIZE 					BT_CONFIG_NET_LWIP_PBUF_POOL_SIZE
#define ARP_TABLE_SIZE 					BT_CONFIG_NET_LWIP_ARP_TABLE_SIZE
#define IP_REASS_MAX_PBUFS				BT_CONFIG_NET_LWIP_IP_REASS_MAX_PBUFS











#endif /* __LWIPOPTS_H__ */
