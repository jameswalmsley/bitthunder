#ifndef _BT_PAGE_H_
#define _BT_PAGE_H_

#include <bt_config.h>
#include <bt_types.h>
#include <collections/bt_list.h>

typedef struct _bt_page_pool {
	struct bt_list_head 	page_head;
	BT_u32					total_size;
	BT_u32					used_size;
} bt_page_pool;

void bt_initialise_pages(void);
void bt_initialise_pages_second_stage(void);
bt_paddr_t bt_initialise_coherent_pages(void);

bt_paddr_t 	bt_page_alloc			(BT_u32 psize);
bt_paddr_t 	bt_page_alloc_aligned	(BT_u32 psize, BT_u32 order);
void 		bt_page_free			(bt_paddr_t paddr, BT_u32 psize);
BT_ERROR 	bt_page_reserve			(bt_paddr_t paddr, BT_u32 psize);

#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
bt_paddr_t 	bt_page_alloc_coherent	(BT_u32 psize);
void 		bt_page_free_coherent	(bt_paddr_t paddr, BT_u32 psize);
BT_ERROR 	bt_page_reserve_coherent(bt_paddr_t paddr, BT_u32 psize);
#endif

#endif
