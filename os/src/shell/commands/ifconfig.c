/**
 *	A Basic ifconfig utility for BitThunder kernel-level shell.
 *
 **/

#include <bitthunder.h>
#include <stdlib.h>
#include <string.h>

static int bt_ifconfig(BT_HANDLE hShell, int argc, char **argv) {

	BT_HANDLE hStdout = BT_ShellGetStdout(hShell);

	BT_u32 i;
	BT_u32 total_interfaces = BT_GetTotalNetworkInterfaces();

	for(i = 0; i < total_interfaces; i++) {
		BT_NET_IF *netif = BT_GetNetifByIndex(i);
		if(!netif) {
			break;
		}

		BT_u8 hwaddr[6];
		BT_NetifGetMacAddress(netif, hwaddr, 6);

		struct bt_phy_linkstate linkstate;
		BT_NetifGetLinkState(netif, &linkstate);

		char *duplex = linkstate.duplex ? "FD" : "HD";

		bt_fprintf(hStdout, "%-5s  Link encap:Ethernet (%d MBit - %s)  HWaddr %02x:%02x:%02x:%02x:%02x\n", netif->name, linkstate.speed, duplex,
				   hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);

		BT_IPADDRESS ip, gw, netmask;
		BT_NetifGetAddress(netif, &ip, &netmask, &gw);

		bt_fprintf(hStdout, "       inet addr:%d.%d.%d.%d  Bcast:%d.%d.%d.%d  Mask: %d.%d.%d.%d\n",
				   ip.a, ip.b, ip.c, ip.d,
				   gw.a, gw.b, gw.c, gw.d,
				   netmask.a, netmask.b, netmask.c, netmask.d);

		char *linkup = linkstate.link ? "UP" : "DOWN";

		bt_fprintf(hStdout, "       %s\n", linkup);
		bt_fprintf(hStdout, "       MAC Type: %s\n", netif->device_name);

	}

	return 0;
}

BT_SHELL_COMMAND_DEF oCommand = {
	.szpName = "ifconfig",
	.pfnCommand = bt_ifconfig,
};
