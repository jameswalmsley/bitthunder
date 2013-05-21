

# lwIP Objects
#LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/bt_lwip.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/def.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/dhcp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/dns.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/init.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/mem.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/memp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/netif.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/pbuf.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/raw.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/stats.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/sys.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/tcp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/tcp_in.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/tcp_out.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/timers.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/udp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/autoip.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/icmp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/igmp.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/inet.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/inet_chksum.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/ip_addr.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/ip.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv4/ip_frag.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP_IPV6) += (BUILD_DIR)os/src/net/lwip/src/core/ipv6/icmp6.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP_IPV6) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv6/inet6.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP_IPV6) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv6/ip6_addr.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP_IPV6) += $(BUILD_DIR)os/src/net/lwip/src/core/ipv6/ip6.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/asn1_dec.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/asn1_enc.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/mib2.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/mib_structs.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/msg_in.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/core/snmp/msg_out.o

LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/api_lib.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/api_msg.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/err.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/netbuf.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/netdb.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/netifapi.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/sockets.o
LWIP_OBJECTS-$(BT_CONFIG_NET_LWIP) += $(BUILD_DIR)os/src/net/lwip/src/api/tcpip.o


$(LWIP_OBJECTS-y): MODULE_NAME="lwIP"
$(LWIP_OBJECTS-y): CFLAGS += -I $(BASE)os/src/net/lwip/src/include/
$(LWIP_OBJECTS-y): CFLAGS += -I $(BASE)os/src/net/lwip/src/include/ipv4/
$(LWIP_OBJECTS-y): CFLAGS += -I $(BASE)os/src/net/lwip/src/include/ipv6/
$(LWIP_OBJECTS-y): CFLAGS += -I $(BASE)os/include/net/lwip/


OBJECTS += $(LWIP_OBJECTS-y)
