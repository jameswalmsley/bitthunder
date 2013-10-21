#ifndef _GEM_BDRING_H_
#define _GEM_BDRING_H_

typedef struct _GEM_BD {
	BT_u32 address;
	#define RX_BD_ADDRESS	0xFFFFFFFC
	#define RX_BD_WRAP		0x00000002
	#define RX_BD_OWNERSHIP	0x00000001
	#define TX_BD_ADDRESS	RX_BD_ADDRESS
	BT_u32 flags;
	#define TX_BD_USED		0x80000000
	#define TX_BD_WRAP 		0x40000000
	#define TX_BD_LAST		0x00008000
	#define TX_BD_LENGTH	0x00003FFF
	#define RX_BD_BROADCAST 0x80000000
	#define RX_BD_MULTICAST	0x40000000
	#define RX_BD_UNICAST 	0x20000000
	#define RX_BD_EXTERNAL 	0x10000000
	#define RX_BD_MATCH_REG	0x06000000
	#define RX_BD_SNAP 		0x01000000
	#define RX_BD_CANON		0x00010000
	#define RX_BD_EOF 		0x00008000
	#define RX_BD_SOF 		0x00004000
	#define RX_BD_LENGTH	0x00001FFF
} GEM_BD;

typedef struct _GEM_BDRING {
	BT_u32  phys_base;
	BT_u32  virt_base;
	BT_u32  virt_end;
	BT_u32  length;
	BT_u32  state;
	#define BDRING_STATE_RUNNING    0x00000001
	BT_u32  bd_size;                                ///< Size of a single BD.

	GEM_BD *free_head;                             ///< Pointer to first BD in the free list.
	GEM_BD *pre_head;                              ///< Pointer to the pre-work group.
	GEM_BD *work_head;                             ///< Pointer to the work group.
	GEM_BD *post_head;                             ///< Pointer to the post work group.

	GEM_BD *bda_restart;                   ///< BDA to load when restarting a channel.

	BT_u32  free_count;
	BT_u32  pre_count;
	BT_u32  work_count;
	BT_u32  post_count;

	BT_u32  total_bds;                              ///< Total number of buffer descriptoers for a channel.
} GEM_BDRING;

typedef struct _GEM_REGS {
	BT_u32 net_ctrl;
	#define NET_CTRL_LOOPEN				0x00000002
	#define NET_CTRL_RXEN				0x00000004
	#define NET_CTRL_TXEN				0x00000008
	#define NET_CTRL_MDEN				0x00000010
	#define NET_CTRL_STATCLR			0x00000020
	#define NET_CTRL_STATINC			0x00000040
	#define NET_CTRL_STATWEN			0x00000080
	#define NET_CTRL_BACKPRESS			0x00000100
	#define NET_CTRL_STARTTX			0x00000200
	#define NET_CTRL_HALTTX				0x00000400
	#define NET_CTRL_PAUSETX			0x00000800
	#define NET_CTRL_ZEROPAUSETX		0x00001000
	#define NET_CTRL_STRRXTS 			0x00008000
	#define NET_CTRL_ENPFCPRIPAUSERX 	0x00010000
	#define NET_CTRL_TXPFCPRIPAUSEFRM 	0x00020000
	#define NET_CTRL_FLUSHNEXTRXDPRMPKT	0x00040000

	BT_u32 net_cfg;
    #define NET_CFG_SPEED 				0x00000001
	#define NET_CFG_FDEN 				0x00000002
	#define NET_CFG_NVLANDISC 			0x00000004
	#define NET_CFG_COPYALLEN 			0x00000010
	#define NET_CFG_BCASTDI 			0x00000020
	#define NET_CFG_MCASTHASHEN 		0x00000040
	#define NET_CFG_UCASTHASHEN			0x00000080
	#define NET_CFG_1536RXEN 			0x00000100
	#define NET_CFG_EXTADDRMATCHEN 		0x00000200
	#define NET_CFG_GIGEEN 				0x00000400
	#define NET_CFG_PCS_SEL 			0x00000800
	#define NET_CFG_RETRY_TEST 			0x00001000
	#define NET_CFG_PAUSEEN 			0x00002000
	#define NET_CFG_RXSOFFS 			0x0000C000
	#define NET_CFG_LENERRDSCRD 		0x00010000
	#define NET_CFG_FCSREM 				0x00020000
	#define NET_CFG_MDCCLKDIV 			0x001C0000
	#define NET_CFG_DBUS_WIDTH 			0x00600000
	#define NET_CFG_PAUSECOPYDI  		0x00800000
	#define NET_CFG_RXCHKSUMEN 			0x01000000
	#define NET_CFG_HDRXEN 				0x02000000
	#define NET_CFG_FCSIGNORE 			0x04000000
	#define NET_CFG_SGMII_EN			0x08000000
	#define NET_CFG_IPDSTRETCH 			0x10000000
	#define NET_CFG_BADPREAMBEN 		0x20000000
	#define NET_IGNORE_IPG_RX_ER 		0x40000000
	#define NET_IGNORE_UNIDIR_EN 		0x80000000

	BT_u32 net_status;
	#define NET_STATUS_MGMT_IDLE		0x00000004

	BT_u32 user_io;
	BT_u32 dma_cfg;
    #define DMA_CFG_BLENGTH 			0x0000001F
	#define DMA_CFG_AHB_ENDIAN_MGM_SWP	0x00000040
	#define DMA_CFG_AHB_ENDIAN_PKT_SWP 	0x00000080
	#define DMA_CFG_RXSIZE 				0x00000300
	#define DMA_CFG_TXSIZE				0x00000400
	#define DMA_CFG_TCPCKSUM 			0x00000800
	#define DMA_CFG_RXBUF 				0x00FF0000
	#define DMA_CFG_DISC_NO_AHB 		0x01000000

	BT_u32 tx_status;
	BT_u32 rx_qbar;
	BT_u32 tx_qbar;
	BT_u32 rx_status;
	#define RX_STAT_BUFFNA				0x00000001
	#define RX_STAT_FRAME_RECD 			0x00000002

	BT_u32 intr_status;
	#define GEM_INT_MGMNT_DONE			0x00000001
	#define GEM_INT_RX_COMPLETE 		0x00000002
	#define GEM_INT_RX_USED 			0x00000004
	#define GEM_INT_TX_USED 			0x00000008
	#define GEM_INT_TX_UNDERRUN 		0x00000010
	#define GEM_INT_EX_LATE_COLLISION 	0x00000020
	#define GEM_INT_AHB_ERR 			0x00000040
	#define GEM_INT_TX_COMPLETE 		0x00000080
	#define GEM_INT_LINK_CHANGE 		0x00000200
	#define GEM_INT_RX_OVERRUN 			0x00000400
	#define GEM_INT_HRESP_NOT_OK 		0x00000800
	#define GEM_INT_PAUSE_NON_ZERO_RX 	0x00001000
	#define GEM_INT_PAUSE_ZERO 			0x00002000
    #define GEM_INT_PAUSE_TX 			0x00004000
	#define GEM_INT_EXT_INTR 			0x00008000
	#define GEM_INT_AUTONEG_COMPLETE 	0x00010000
	#define GEM_INT_ALL_MASK			0x03FC7FFE /* Everything except MDIO */


	BT_u32 intr_enable;
	BT_u32 intr_disable;
	BT_u32 intr_mask;

	BT_u32 phy_maint;
	#define PHY_MAINT_DATA				0x0000FFFF
	#define PHY_MAINT_REG_ADDR 			0x007C0000
	#define PHY_MAINT_PHY_ADDR 			0x0F800000
	#define PHY_MAINT_OP 				0x30000000
	#define PHY_MAINT_W_MASK			0x10000000
	#define PHY_MAINT_R_MASK			0x20000000

	BT_u32 rx_pauseq;
	BT_u32 tx_pauseq;

	BT_STRUCT_RESERVED_u32(0, 0x3C, 0x80);

	BT_u32 hash_bot;
	BT_u32 hash_top;
	BT_u32 spec_addr1_bot;
	BT_u32 spec_addr1_top;
	BT_u32 spec_addr2_bot;
	BT_u32 spec_addr2_top;
	BT_u32 spec_addr3_bot;
	BT_u32 spec_addr3_top;
	BT_u32 spec_addr4_bot;
	BT_u32 spec_addr4_top;
	BT_u32 type_id_match1;
	BT_u32 type_id_match2;
	BT_u32 type_id_match3;
	BT_u32 type_id_match4;
	BT_u32 wake_on_lan;
	BT_u32 ipg_stretch;
	BT_u32 stacked_vlan;
	BT_u32 tx_pfc_pause;
	BT_u32 spec_addr1_mask_bot;
	BT_u32 spec_addr1_make_top;

	BT_STRUCT_RESERVED_u32(1, 0xCC, 0xFC);

	BT_u32 module_id;
	BT_u32 octets_tx_bot;
	BT_u32 octets_tx_top;
	BT_u32 frames_tx;
	BT_u32 broadcast_frames_tx;
	BT_u32 multi_frames_tx;
	BT_u32 pause_frames_tx;
	BT_u32 frames_64b_tx;
	BT_u32 frames_65to127b_tx;
	BT_u32 frames_128to255b_tx;
	BT_u32 frames_256to511b_tx;
	BT_u32 frames_512to1023_tx;
	BT_u32 frames_1024to1518_tx;
	BT_u32 tx_under_runs;
	BT_u32 single_collisn_frames;
	BT_u32 multi_collisn_frames;
	BT_u32 excessive_collisns;
	BT_u32 late_collisns;
	BT_u32 deferred_tx_frames;
	BT_u32 carrier_sense_errs;
	BT_u32 octets_rx_bot;
	BT_u32 octets_rx_top;
	BT_u32 frames_rx;
	BT_u32 bdcast_frames_rx;
	BT_u32 multi_frames_rx;
	BT_u32 pause_rx;
	BT_u32 frames_64b_rx;
	BT_u32 frames_65to127b_rx;
	BT_u32 frames_128to255b_rx;
	BT_u32 frames_256to511b_rx;
	BT_u32 frames_512to1023_rx;
	BT_u32 frames_1024to1518_rx;
	BT_u32 undersz_rx;
	BT_u32 oversz_rx;
	BT_u32 jab_rx;
	BT_u32 fcs_errors;
	BT_u32 length_field_errors;
	BT_u32 rx_symbol_errors;
	BT_u32 align_errors;
	BT_u32 rx_resource_errors;
	BT_u32 rx_overrun_errors;
	BT_u32 ip_hdr_csum_errors;
	BT_u32 tcp_csum_errors;
	BT_u32 udp_csum_errors;

	BT_STRUCT_RESERVED_u32(2, 0x1B0, 0x1C8);

	BT_u32 timer_strobe_s;
	BT_u32 timer_stobe_ns;
	BT_u32 timer_s;
	BT_u32 timer_ns;
	BT_u32 timer_adjust;
	BT_u32 timer_incr;
	BT_u32 ptp_tx_s;
	BT_u32 ptp_tx_ns;
	BT_u32 ptp_rx_s;
	BT_u32 ptp_rx_ns;
	BT_u32 ptp_peer_tx_s;
	BT_u32 ptp_peer_tx_ns;
	BT_u32 ptp_peer_rx_s;
	BT_u32 ptp_peer_rx_ns;

	BT_STRUCT_RESERVED_u32(3, 0x1FC, 0x284);

	BT_u32 design_cfg2;
	BT_u32 design_cfg3;
	BT_u32 design_cfg4;
	BT_u32 design_cfg5;

} GEM_REGS;

#endif
