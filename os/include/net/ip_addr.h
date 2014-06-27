/* This is the aligned version of ip_addr_t,
   used as local variable, on the stack, etc. */
struct ip_addr {
  BT_u32 addr;
};

typedef struct ip_addr ip_addr_t;

/** IP_ADDR_ can be used as a fixed IP address
 *  for the wildcard and the broadcast address
 */
#define IP_ADDR_ANY         ((ip_addr_t *)&ip_addr_any)
#define IP_ADDR_BROADCAST   ((ip_addr_t *)&ip_addr_broadcast)

/** 255.255.255.255 */
#define IPADDR_NONE         ((BT_u32)0xffffffffUL)
/** 127.0.0.1 */
#define IPADDR_LOOPBACK     ((BT_u32)0x7f000001UL)
/** 0.0.0.0 */
#define IPADDR_ANY          ((BT_u32)0x00000000UL)
/** 255.255.255.255 */
#define IPADDR_BROADCAST    ((BT_u32)0xffffffffUL)

/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */
#define IP_CLASSA(a)        ((((BT_u32)(a)) & 0x80000000UL) == 0)
#define IP_CLASSA_NET       0xff000000
#define IP_CLASSA_NSHIFT    24
#define IP_CLASSA_HOST      (0xffffffff & ~IP_CLASSA_NET)
#define IP_CLASSA_MAX       128

#define IP_CLASSB(a)        ((((BT_u32)(a)) & 0xc0000000UL) == 0x80000000UL)
#define IP_CLASSB_NET       0xffff0000
#define IP_CLASSB_NSHIFT    16
#define IP_CLASSB_HOST      (0xffffffff & ~IP_CLASSB_NET)
#define IP_CLASSB_MAX       65536

#define IP_CLASSC(a)        ((((BT_u32)(a)) & 0xe0000000UL) == 0xc0000000UL)
#define IP_CLASSC_NET       0xffffff00
#define IP_CLASSC_NSHIFT    8
#define IP_CLASSC_HOST      (0xffffffff & ~IP_CLASSC_NET)

#define IP_CLASSD(a)        (((BT_u32)(a) & 0xf0000000UL) == 0xe0000000UL)
#define IP_CLASSD_NET       0xf0000000          /* These ones aren't really */
#define IP_CLASSD_NSHIFT    28                  /*   net and host fields, but */
#define IP_CLASSD_HOST      0x0fffffff          /*   routing needn't know. */
#define IP_MULTICAST(a)     IP_CLASSD(a)

#define IP_EXPERIMENTAL(a)  (((BT_u32)(a) & 0xf0000000UL) == 0xf0000000UL)
#define IP_BADCLASS(a)      (((BT_u32)(a) & 0xf0000000UL) == 0xf0000000UL)

#define IP_LOOPBACKNET      127                 /* official! */


#if BYTE_ORDER == BIG_ENDIAN
/** Set an IP address given by the four byte-parts */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((BT_u32)((a) & 0xff) << 24) | \
                         ((BT_u32)((b) & 0xff) << 16) | \
                         ((BT_u32)((c) & 0xff) << 8)  | \
                          (BT_u32)((d) & 0xff)
#else
/** Set an IP address given by the four byte-parts.
    Little-endian version that prevents the use of htonl. */
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((BT_u32)((d) & 0xff) << 24) | \
                         ((BT_u32)((c) & 0xff) << 16) | \
                         ((BT_u32)((b) & 0xff) << 8)  | \
                          (BT_u32)((a) & 0xff)
#endif
