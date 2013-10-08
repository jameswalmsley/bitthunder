#ifndef _BT_PAGE_H_
#define _BT_PAGE_H_

#include <bt_config.h>
#include <bt_types.h>
#include <collections/bt_list.h>

typedef struct _BT_PAGE {
	struct bt_list_head		list;
	BT_u32					size;
	BT_u32 					flags;
    #define BT_PAGE_USED			0x00000001	///< Flags if this page is free or not.
    #define BT_PAGE_HEAD 			0x00000002	///< Node is head of an allocated block.
    #define BT_PAGE_RESERVED		0x80000000	///< Page has been removed from PAGE allocator.
} BT_PAGE;

void bt_initialise_pages(void);
bt_paddr_t bt_page_alloc(BT_u32 psize);
void bt_page_free(bt_paddr_t paddr, BT_u32 size);

#endif
