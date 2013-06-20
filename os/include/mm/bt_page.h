#ifndef _BT_PAGE_H_
#define _BT_PAGE_H_

#include <bt_config.h>
#include <bt_types.h>
#include <collections/bt_list.h>
#include <mm/bt_mm.h>


typedef struct _BT_PAGE {
	struct bt_list_head		list;
	BT_u32					size;
	BT_u32 					flags;
    #define BT_PAGE_USED	0x00000001	///< Flags if this page is free or not.
    #define BT_PAGE_HEAD 	0x00000002	///< Node is head of an allocated block.
} BT_PAGE;








#endif
