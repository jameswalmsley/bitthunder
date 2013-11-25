/**
 *	Drivers/Modules need to be able to declare primitive resources which they can claim.
 *
 *
 **/
#ifndef _BT_RESOURCE_H_
#define _BT_RESOURCE_H_

/**
 *	Here we define a structure that can describe device resources or parameters.
 *	A union and struct is used to remove redundancy from the in-memory or in-rom objects.
 **/
typedef struct _BT_RESOURCE {
	union {
		struct {
			BT_u32	ulStart;
			BT_u32	ulEnd;
		};
		const char 	*szpName;
		void		*pParam;
		BT_u32		ulConfigFlags;
	};
	BT_u32			ulFlags;
	//struct _BT_RESOURCE *parent, *sibling, *child;
} BT_RESOURCE;

#define BT_RESOURCE_TYPE_BITS	0x0007FF00
#define BT_RESOURCE_IO			0x00000100
#define BT_RESOURCE_MEM			0x00000200
#define BT_RESOURCE_IRQ			0x00000400
#define BT_RESOURCE_DMA			0x00000800
#define BT_RESOURCE_ENUM 		0x00001000		///< Resource descriptor used for numbering devices easily.
#define BT_RESOURCE_STRING		0x00002000		///< A string resource, e.g. a sub-driver.
#define BT_RESOURCE_PARAM		0x00004000		///< Void * pParam resource, e.g. some operations.
#define BT_RESOURCE_FLAGS		0x00008000		///< A BT_u32 type for passing parameter flags.
#define BT_RESOURCE_BUSID		0x00010000		///< A BusID.
#define BT_RESOURCE_INTEGER		0x00020000		///< Generic Integer Value
#define BT_RESOURCE_NUM_CS		0x00040000

#define BT_RESOURCE_TYPE(x)		(x & BT_RESOURCE_TYPE_BITS)

const BT_RESOURCE *BT_GetResource(const BT_RESOURCE *pResources, BT_u32 ulTotalResources, BT_u32 ulType, BT_u32 ulNum);

#endif
