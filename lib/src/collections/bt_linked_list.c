/**
 *	BitThunder Singly linked-list structure.
 *
 **/

#include <bitthunder.h>
#include <collections/bt_linked_list.h>


BT_BOOL BT_ListInitialised(BT_LIST *pL) {
	return (pL->hListMutex != NULL);
}

BT_ERROR BT_ListInit(BT_LIST *pL) {
	pL->pStart = NULL;
	pL->ulItems = 0;
	pL->hListMutex = NULL;
	return BT_ERR_NONE;
}

BT_ERROR BT_ListDestroy(BT_LIST *pL) {
	return BT_ERR_NONE;
}

BT_LIST_ITEM *BT_ListGetHead(BT_LIST *pL) {
	return pL->pStart;
}

BT_LIST_ITEM *BT_ListGetNext(BT_LIST_ITEM *p) {
	return (BT_LIST_ITEM *) p->pNext;
}

BT_ERROR BT_ListAddItem(BT_LIST *pL, BT_LIST_ITEM *p) {
	BT_LIST_ITEM *pItem = pL->pStart;
	if(!pItem) {
		pL->pStart = p;
	} else {
		while(pItem->pNext) {
			pItem = (BT_LIST_ITEM *) pItem->pNext;
		}
	}

	p->pNext = NULL;
	p->pList = pL;

	if(pItem) {
		pItem->pNext = p;
	}

	pL->ulItems += 1;

	return BT_ERR_NONE;
}

BT_ERROR BT_ListRemoveItem(BT_LIST *pL, BT_LIST_ITEM *p) {
	BT_LIST_ITEM *pItem = pL->pStart;

	if(pItem == p) {
		pL->pStart = p->pNext;
		return BT_ERR_NONE;
	}

	while(pItem && pItem->pNext) {
		if(pItem->pNext == p) {
			pItem->pNext = p->pNext;
			return BT_ERR_NONE;
		}
	}

	return BT_ERR_GENERIC;
}
