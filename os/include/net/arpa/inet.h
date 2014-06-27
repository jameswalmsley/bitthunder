

#ifdef BT_CONFIG_BIG_ENDIAN
#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)
#define PP_HTONS(x) (x)
#define PP_NTOHS(x) (x)
#define PP_HTONL(x) (x)
#define PP_NTOHL(x) (x)
#endif

#ifdef BT_CONFIG_LITTLE_ENDIAN
#define htons(x)	bt_cpu_to_be16(x)
#define ntohs(x)	bt_be16_to_cpu(x)
#define htonl(x)	bt_cpu_to_be32(x)
#define ntohl(x)	bt_be32_to_cpu(x)
#define PP_HTONS(x) htons(x)
#define PP_NTOHS(x) ntohs(x)
#define PP_HTONL(x) htonl(x)
#define PP_NTOHL(x) ntohl(x)
#endif
