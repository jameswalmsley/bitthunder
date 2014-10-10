/**
 *	BitThunder Heap Implementation.
 *
 *	This is the Kernel-Mode global heap implementation.
 *
 **/
#include <bitthunder.h>

extern void * _heap_start;
extern void * _heap_end;

typedef struct _BT_HEAP_BLOCK {
	struct _BT_HEAP_BLOCK  *pNextBlock;
	BT_u32					ulSize;
} BT_HEAP_BLOCK;

static BT_HEAP_BLOCK pStart, *pEnd = NULL;
static BT_u32	g_ulBytesRemaining = 0;

static void BT_InitialiseHeap() {

	pStart.pNextBlock = (BT_HEAP_BLOCK *) &_heap_start;
	pStart.ulSize = 0;

	pEnd = (BT_HEAP_BLOCK *) ((char *) &_heap_end) - sizeof(BT_HEAP_BLOCK);
	pEnd->pNextBlock = NULL;
	pEnd->ulSize = 0;

	BT_HEAP_BLOCK *pFirstFreeBlock = (BT_HEAP_BLOCK *) &_heap_start;
	pFirstFreeBlock->pNextBlock = pEnd;
	pFirstFreeBlock->ulSize = (BT_u32)(&_heap_end) - (BT_u32)(&_heap_start) - sizeof(BT_HEAP_BLOCK);

	g_ulBytesRemaining = pFirstFreeBlock->ulSize;
}

void *BT_kMalloc(BT_u32 ulSize) {

	void *p = NULL;

	// CRITICAL SECTION
	BT_kEnterCritical();
	{

		/*
		 *	Ensure we have an initialised heap.
		 */
		if(!pEnd) {
			BT_InitialiseHeap();
		}

		if(!ulSize) {
			goto complete;
		}

		ulSize += sizeof(BT_HEAP_BLOCK);

		// Force alignment of the requested size.
		if((ulSize & 0x7)) {
			ulSize += (8 - (ulSize & 0x7));	// Force size onto an aligned boundary! (8 byte aligned).
		}

		if(ulSize > g_ulBytesRemaining) {
			goto complete;
		}

		// Now we have a sane request, we must look at our free list and see if we can satisfy it?

		BT_HEAP_BLOCK *pPrevious 	= &pStart;
		BT_HEAP_BLOCK *pBlock 		= pStart.pNextBlock;

		while((pBlock->ulSize < ulSize) && (pBlock->pNextBlock != NULL)) {
			pPrevious = pBlock;
			pBlock = pBlock->pNextBlock;
		}

		if(pBlock == pEnd) {
			goto complete;
		}

		// Memory to return, is the previous block's next pointer, plus the bytes
		// For the new block descriptor.
		p = (void *) (((unsigned char *) pPrevious->pNextBlock) + sizeof(BT_HEAP_BLOCK));


		// Remove it from the free block list

		pPrevious->pNextBlock = pBlock->pNextBlock;

		// In case the allocated space is larger than required, we can split it, and
		// create an extra free area.

		if((pBlock->ulSize - ulSize) > (sizeof(BT_HEAP_BLOCK) * 2)) {
			BT_HEAP_BLOCK *pNewBlock = (void *) (((unsigned char *) pBlock) + ulSize);
			pNewBlock->pNextBlock = pBlock->pNextBlock;
			pNewBlock->ulSize = pBlock->ulSize - ulSize;

			pBlock->ulSize = ulSize;
			pBlock->pNextBlock = NULL;

			pPrevious->pNextBlock = pNewBlock;
		}

		g_ulBytesRemaining -= pBlock->ulSize;
	}
	// END CRITICAL SECTION.

complete:
	BT_kExitCritical();
	return p;
}
BT_EXPORT_SYMBOL(BT_kMalloc);


void BT_kFree(void *p) {
	BT_HEAP_BLOCK *pBlock = (BT_HEAP_BLOCK *) p;
	if(!p) {
		return;
	}

	BT_kEnterCritical();

	pBlock--;	// Pointer should have a HEAP_BLOCK behind it, therefore just decrement the pointer.

	g_ulBytesRemaining += pBlock->ulSize;

	BT_HEAP_BLOCK *pItem;

	for(pItem = &pStart; pItem->pNextBlock < pBlock; pItem = pItem->pNextBlock) {
		;
	}

	// Can we merge the previous block with pBlock to be freed.
	if(((unsigned char *) (pItem) + pItem->ulSize) == (unsigned char *) pBlock) {
		pItem->ulSize += pBlock->ulSize;
		pBlock = pItem;
	}

	if(((unsigned char *) (pBlock) + pBlock->ulSize) == (unsigned char *) pItem->pNextBlock) {
		if(pItem->pNextBlock != pEnd) {
			pBlock->ulSize += pItem->pNextBlock->ulSize;
			pBlock->pNextBlock = pItem->pNextBlock->pNextBlock;
		} else {
			pBlock->pNextBlock = pEnd;
		}
	} else {
		pBlock->pNextBlock = pItem->pNextBlock;
	}

	if(pItem != pBlock) {
		pItem->pNextBlock = pBlock;
	}

	BT_kExitCritical();
}
BT_EXPORT_SYMBOL(BT_kFree);

void *BT_kRealloc(void *p, BT_u32 ulSize) {
	void *n = NULL;

	// CRITICAL SECTION
	BT_kEnterCritical();
	{
		if((p) && (ulSize)) {
			n = BT_kMalloc(ulSize);
			if (n)
			{
				BT_HEAP_BLOCK * pOld = (BT_HEAP_BLOCK *)p;
				pOld--;
				pOld->ulSize -= sizeof(BT_HEAP_BLOCK);
				if (ulSize > pOld->ulSize)
					memcpy(n, p, pOld->ulSize);
				else
					memcpy(n, p, ulSize);
			}
		}
		else if((p) && (!ulSize)) {
			BT_kFree(p);
		}
		else if((!p) && (ulSize)) {
			n = BT_kMalloc(ulSize);
		}
	}
	BT_kExitCritical();

	return n;
}
BT_EXPORT_SYMBOL(BT_kRealloc);
