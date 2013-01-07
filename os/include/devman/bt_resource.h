/**
 *	Drivers/Modules need to be able to declare primitive resources which they can claim.
 *
 *
 **/
#ifndef _BT_RESOURCE_H_
#define _BT_RESOURCE_H_

typedef struct _BT_RESOURCE {
	BT_u32			ulStart;
	BT_u32			ulEnd;
	const char 	   *szpName;
	BT_u32			ulFlags;
	struct _BT_RESOURCE *parent, *sibling, *child;
} BT_RESOURCE;


#define BT_RESOURCE_TYPE_BITS	0x00000F00
#define BT_RESOURCE_IO			0x00000100
#define BT_RESOURCE_MEM			0x00000200
#define BT_RESOURCE_IRQ			0x00000400
#define BT_RESOURCE_DMA			0x00000800

#define BT_RESOURCE_TYPE(x)		(x & BT_RESOURCE_TYPE_BITS)

#endif
