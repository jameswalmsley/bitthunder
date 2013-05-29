#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <bt_config.h>

/* Prevent having to link sys_arch.c (we don't test the API layers in unit tests) */
#define NO_SYS                          0
#define LWIP_NETCONN                    0
#define LWIP_SOCKET                     0
#define	SYS_LIGHTWEIGHT_PROT			1

#define LWIP_TCP						BT_CONFIG_USE_TCP
#define LWIP_UDP						BT_CONFIG_USE_UDP
#define LWIP_DHCP						BT_CONFIG_USE_DHCP
#define LWIP_IGMP						BT_CONFIG_USE_IGMP
#define LWIP_NETIF_HOSTNAME				BT_CONFIG_USE_DHCP


#define TCP_MSS							1500
#define MEM_SIZE						16000
#define MEM_ALIGNMENT                   4
#define TCP_SND_QUEUELEN                40
#define MEMP_NUM_TCP_SEG                TCP_SND_QUEUELEN
#define TCP_SND_BUF                     (3 * TCP_MSS)
#define TCP_WND                         (2 * TCP_MSS)

//*****************************************************************************
// ---------- Pbuf options ----------
//*****************************************************************************
#define ETH_PAD_SIZE                    2		// to ensure 32 bit alignment


/* Minimal changes to opt.h required for etharp unit tests: */
#define ETHARP_SUPPORT_STATIC_ENTRIES   1


//#define	LWIP_COMPAT_MUTEX				1

#define TCPIP_MBOX_SIZE                 10
#define DEFAULT_RAW_RECVMBOX_SIZE       10
#define DEFAULT_UDP_RECVMBOX_SIZE       10
#define DEFAULT_TCP_RECVMBOX_SIZE       10
#define DEFAULT_ACCEPTMBOX_SIZE         10




#endif /* __LWIPOPTS_H__ */
