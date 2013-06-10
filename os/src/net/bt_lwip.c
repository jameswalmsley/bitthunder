/**
 *	LWIP TCP/IP stack - for BitThunder.
 *
 **/

#include <net/bt_lwip.h>
#include <collections/bt_fifo.h>
#include <lwip/sys.h>
#include <lwip/stats.h>
#include <netif/etharp.h>
#include <lwip/tcpip.h>

BT_DEF_MODULE_NAME			("LWIP TCP/IP stack")
BT_DEF_MODULE_DESCRIPTION	("BitThunder TCP/IP stack plugin for LWIP")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.co.at")


/**
 * Setup processing for PTP (IEEE-1588).
 *
 */
#if LWIP_PTPD
extern void lwIPHostGetTime(u32_t *time_s, u32_t *time_ns);
#endif

/**
 * Pop a pbuf packet from a pbuf packet fifo
 *
 * @return pointer to pbuf packet if available, NULL otherwise.
 */
static struct pbuf * read_packet(BT_NET_IF *pIF) {
	void *pBuf = NULL;
	BT_ERROR Error = BT_ERR_NONE;

	SYS_ARCH_DECL_PROTECT(lev);

	/**
	* This entire function must run within a "critical section" to preserve
	* the integrity of the transmit pbuf queue.
	*
	*/
	SYS_ARCH_PROTECT(lev);

	if (!BT_FifoIsEmpty(pIF->hTxFifo, &Error)) {
		BT_FifoRead(pIF->hTxFifo, 1, &pBuf, &Error);
	}

	/* Return to prior interrupt state and return the pbuf pointer. */
	SYS_ARCH_UNPROTECT(lev);
	return((struct pbuf*)pBuf);
}

/**
 * Push a pbuf packet onto a pbuf packet queue
 *
 * @param p is the pbuf to push onto the packet queue.
 * @param q is the packet queue.
 *
 * @return 1 if successful, 0 if q is full.
 */
static BT_u32 write_packet(BT_NET_IF *pIF, struct pbuf *pBuf) {
	SYS_ARCH_DECL_PROTECT(lev);
	BT_u32 ulret;
	BT_ERROR Error = BT_ERR_NONE;

	/**
	* This entire function must run within a "critical section" to preserve
	* the integrity of the transmit pbuf queue.
	*/
	SYS_ARCH_PROTECT(lev);

	//BT_u32 pPointer = (BT_u32)pBuf;
	void * pPointer = (void*)pBuf;

	ulret = BT_FifoWrite(pIF->hTxFifo, 1, &pPointer, &Error);

	/* Return to prior interrupt state and return the pbuf pointer. */
	SYS_ARCH_UNPROTECT(lev);
	return(ulret);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf might be
 * chained.
 *
 * @param netif the lwIP network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 * @note This function MUST be called with interrupts disabled or with the
 *       lwIP Ethernet transmit fifo protected.
 */
static err_t lwIPif_transmit(struct netif *netif, struct pbuf *p) {
	BT_NET_IF *pIF = (BT_NET_IF*)netif->state;
	int iBuf;
	unsigned char *pucBuf;
	unsigned long *pulBuf;
	struct pbuf *q;
	int iGather;
	unsigned long ulGather;
	unsigned char *pucGather;

	/**
	* Fill in the first two bytes of the payload data (configured as padding
	* with ETH_PAD_SIZE = 2) with the total length of the payload data
	* (minus the Ethernet MAC layer header).
	*
	*/
	*((unsigned short *)(p->payload)) = p->tot_len - 16;

	/* Initialize the gather register. */
	iGather = 0;
	pucGather = (unsigned char *)&ulGather;
	ulGather = 0;

	/* Copy data from the pbuf(s) into the TX Fifo. */
	for(q = p; q != NULL; q = q->next) {
		/* Intialize a char pointer and index to the pbuf payload data. */
		pucBuf = (unsigned char *)q->payload;
		iBuf = 0;

		/**
		* If the gather buffer has leftover data from a previous pbuf
		* in the chain, fill it up and write it to the Tx FIFO.
		*
		*/
		while((iBuf < q->len) && (iGather != 0)) {
			/* Copy a byte from the pbuf into the gather buffer. */
			pucGather[iGather] = pucBuf[iBuf++];

			/* Increment the gather buffer index modulo 4. */
			iGather = ((iGather + 1) % 4);
		}

		/**
		* If the gather index is 0 and the pbuf index is non-zero,
		* we have a gather buffer to write into the Tx FIFO.
		*
		*/
		if((iGather == 0) && (iBuf != 0)) {
			pIF->pOps->pfnWrite(pIF->hIF, 1, (void*)&ulGather);
			ulGather = 0;
		}

		/* Initialze a long pointer into the pbuf for 32-bit access. */
		pulBuf = (unsigned long *)&pucBuf[iBuf];

		/**
		* Copy words of pbuf data into the Tx FIFO, but don't go past
		* the end of the pbuf.
		*
		*/
		pIF->pOps->pfnWrite(pIF->hIF, q->len/4, (void*)pulBuf);
		pulBuf += q->len/4;

		/**
		* Check if leftover data in the pbuf and save it in the gather
		* buffer for the next time.
		*
		*/
		while(iBuf < q->len) {
			/* Copy a byte from the pbuf into the gather buffer. */
			pucGather[iGather] = pucBuf[iBuf++];

			/* Increment the gather buffer index modulo 4. */
			iGather = ((iGather + 1) % 4);
		}
	}

	/* Send any leftover data to the FIFO. */
	pIF->pOps->pfnWrite(pIF->hIF, 1, (void*)&ulGather);

	/* Wakeup the transmitter. */
	pIF->pOps->pfnSendFrame(pIF->hIF);

	/* Dereference the pbuf from the queue. */
	pbuf_free(p);

	LINK_STATS_INC(link.xmit);

	return(ERR_OK);
}

/**
 * This function with either place the packet into the lwIP transmit fifo,
 * or will place the packet in the interface PBUF Queue for subsequent
 * transmission when the transmitter becomes idle.
 *
 * @param netif the lwIP network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 */
static err_t lwip_output(struct netif *netif, struct pbuf *p) {
	BT_NET_IF *pIF = (BT_NET_IF*)netif->state;
	BT_ERROR Error = BT_ERR_NONE;
	SYS_ARCH_DECL_PROTECT(lev);

	/**
	* This entire function must run within a "critical section" to preserve
	* the integrity of the transmit pbuf queue.
	*/
	SYS_ARCH_PROTECT(lev);

	/**
	* Bump the reference count on the pbuf to prevent it from being
	* freed till we are done with it.
	*/
	pbuf_ref(p);

	/**
	* If the transmitter is idle, and there is nothing on the queue,
	* send the pbuf now.
	*/
	if (BT_FifoIsEmpty(pIF->hTxFifo, &Error) && (pIF->pOps->pfnTxFifoReady(pIF->hIF, &Error))) {
		lwIPif_transmit(netif, p);
	}

	/* Otherwise place the pbuf on the transmit queue. */
	else {
		/* Add to transmit packet queue */
		if(!write_packet(pIF, p)) {
			/* if no room on the queue, free the pbuf reference and return error. */
			pbuf_free(p);
			SYS_ARCH_UNPROTECT(lev);
			return (ERR_MEM);
		}
	}

	/* Return to prior interrupt state and return. */
	SYS_ARCH_UNPROTECT(lev);
	return ERR_OK;
}

/**
 * This function will read a single packet from the lwIP ethernet
 * interface, if available, and return a pointer to a pbuf.  The timestamp
 * of the packet will be placed into the pbuf structure.
 *
 * @param netif the lwIP network interface structure for this ethernetif
 * @return pointer to pbuf packet if available, NULL otherswise.
 */
static struct pbuf * lwip_receive(struct netif *netif) {
	BT_NET_IF *pIF = (BT_NET_IF *)netif->state;
	BT_ERROR Error = BT_ERR_NONE;

	struct pbuf *p, *q;
	BT_u32 ullen;
	unsigned long *ptr;
	#if LWIP_PTPD
	u32_t time_s, time_ns;

	/* Get the current timestamp if PTPD is enabled */
	lwIPHostGetTime(&time_s, &time_ns);
	#endif


	/* Check if a packet is available, if not, return NULL packet. */
	BT_u32 ulTemp = pIF->pOps->pfnDataReady(pIF->hIF, &Error);
	ullen = ulTemp & 0xFFFF;

	if (ullen == 0) {
		return(NULL);
	}

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, ullen, PBUF_POOL);

	/* If a pbuf was allocated, read the packet into the pbuf. */
	if(p != NULL) {
		/* Place the first word into the first pbuf location. */
		*(unsigned long *)p->payload = ulTemp;
		p->payload = (char *)(p->payload) + 4;
		p->len -= 4;

		/* Process all but the last buffer in the pbuf chain. */
		q = p;
		while(q != NULL) {
			/* Setup a byte pointer into the payload section of the pbuf. */
			ptr = q->payload;

			/**
			* Read data from FIFO into the current pbuf
			* (assume pbuf length is modulo 4)
			*
			*/
			pIF->pOps->pfnRead(pIF->hIF, q->len, ptr);
			ptr += q->len/4;

			/* Link in the next pbuf in the chain. */
			q = q->next;
		}

		/* Restore the first pbuf parameters to their original values. */
		p->payload = (char *)(p->payload) - 4;
		p->len += 4;

		/* Adjust the link statistics */
		LINK_STATS_INC(link.recv);

		#if LWIP_PTPD
		/* Place the timestamp in the PBUF */
		p->time_s = time_s;
		p->time_ns = time_ns;
		#endif
	}

	/* If no pbuf available, just drain the RX fifo. */
	else {
		Error = pIF->pOps->pfnDropFrame(pIF->hIF, ullen-4);

		/* Adjust the link statistics */
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
	}

	return(p);
}

/**
 * Process tx and rx packets at the low-level interrupt.
 *
 * Should be called from the lwIP Ethernet Interrupt Handler.  This
 * function will read packets from the lwIP Ethernet fifo and place them
 * into a pbuf queue.  If the transmitter is idle and there is at least one packet
 * on the transmit queue, it will place it in the transmit fifo and start the
 * transmitter.
 *
 */
void bt_lwip_process(BT_NETIF_PRIV *pIF) {
	struct netif *netif = &pIF->netif;
	struct pbuf *p;
	BT_ERROR Error = BT_ERR_NONE;

	/**
	* Process the transmit and receive queues as long as there is receive
	* data available
	*
	*/
	p = lwip_receive(netif);
	while(p != NULL) {
		/* process the packet */
		if(tcpip_input(p, netif)!=ERR_OK) {
			/* drop the packet */
			LWIP_DEBUGF(NETIF_DEBUG, ("lwIPif_input: input error\n"));
			pbuf_free(p);

			/* Adjust the link statistics */
			LINK_STATS_INC(link.memerr);
			LINK_STATS_INC(link.drop);
		}

		/* Check if TX fifo is empty and packet available */
		if(pIF->base.pOps->pfnTxFifoReady(pIF->base.hIF, &Error)) {
			p = read_packet(&pIF->base);
			if(p != NULL) {
				lwIPif_transmit(netif, p);
			}
		}

		/* Read another packet from the RX fifo */
		p = lwip_receive(netif);
	}

	/* One more check of the transmit queue/fifo */
	if(pIF->base.pOps->pfnTxFifoReady(pIF->base.hIF, &Error)) {
		p = read_packet(&pIF->base);
		if(p != NULL) {
			lwIPif_transmit(netif, p);
		}
	}
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function lwIPif_hwinit() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwIP network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
BT_ERROR bt_lwip_netif_init(BT_NETIF_PRIV *pIF) {
	struct netif *netif = &pIF->netif;
	BT_ERROR Error = BT_ERR_NONE;

	#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "BitThunder";
	#endif /* lwIP_NETIF_HOSTNAME */

	/*
	* Initialize the snmp variables and counters inside the struct netif.
	* The last argument should be replaced with your link speed, in units
	* of bits per second.
	*/
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 1000000);

	netif->name[0] = 'e';
	netif->name[1] = (pIF->base.ulID % 9) + '0';
	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */
	netif->output = etharp_output;
	netif->linkoutput = lwip_output;

	/* initialize the hardware */

	pIF->base.pOps->pfnInitialise(pIF->base.hIF);

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	pIF->base.pOps->pfnGetMACAddr(pIF->base.hIF, &(netif->hwaddr[0]), ETHARP_HWADDR_LEN);

	/* maximum transfer unit */
	netif->mtu = pIF->base.pOps->pfnGetMTU(pIF->base.hIF, &Error);

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;


	return BT_ERR_NONE;
}


