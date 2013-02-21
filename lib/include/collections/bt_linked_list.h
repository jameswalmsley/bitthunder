/**
 *	Linked list structure definition.
 **/

#ifndef _BT_LINKED_LIST_H_
#define _BT_LINKED_LIST_H_

#include <bt_types.h>

struct _BT_LIST;

typedef struct _BT_LIST_ITEM {
	struct _BT_LIST_ITEM 	*pNext;
	struct _BT_LIST	  	*pList;
} BT_LIST_ITEM;

typedef struct _BT_LIST {
	struct _BT_LIST_ITEM  *pStart;
	BT_u32 					ulItems;
	BT_HANDLE				hListMutex;
} BT_LIST;

BT_ERROR BT_ListInit(BT_LIST *pL);
BT_ERROR BT_ListDestroy(BT_LIST *pL);
BT_LIST_ITEM *BT_ListGetHead(BT_LIST *pL);
BT_LIST_ITEM *BT_ListGetNext(BT_LIST_ITEM *p);
BT_ERROR BT_ListAddItem(BT_LIST *pL, BT_LIST_ITEM *pItem);
BT_ERROR BT_ListRemoveItem(BT_LIST *pL, BT_LIST_ITEM *pItem);

#endif
