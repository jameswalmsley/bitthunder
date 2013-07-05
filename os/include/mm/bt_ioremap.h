#ifndef _BT_IOREMAP_H_
#define _BT_IOREMAP_H_

#include <bt_config.h>
#include <bt_types.h>
#include <collections/bt_list.h>

typedef struct _BT_IOMAP {
	struct bt_list_head 	list;
	void				   *addr;	///< Virtual Memory Address of this mapping.
	BT_PHYS_ADDR			phys;	///< Physical address of this mapping.
	BT_u32					size;	///< Size of the mapping.
	BT_u32					type;
	#define BT_IOMAP_TYPE_RESERVED		0x00000001
    #define BT_IOMAP_TYPE_KERNEL_MAP	0x00000002
} BT_IOMAP;

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
void *bt_ioremap(void *phys_addr, BT_u32 size);
#else
#define bt_ioremap(x, y)	x
#endif

#endif
