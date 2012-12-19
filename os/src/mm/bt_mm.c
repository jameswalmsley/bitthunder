/**
 *	OS Version of the memory manager.
 *
 *
 **/

#include <bitthunder.h>
#include <mm/bt_heap.h>
#include <string.h>


void *BT_Calloc(BT_u32 ulSize) {
	void *p = BT_kMalloc(ulSize);
	if(p) {
		memset(p, 0, ulSize);
	}

	return p;
}


/**
 *	Force Malloc and free to be forced onto our own APIs.
 *
 **/
void *malloc(int size) {
	return BT_kMalloc(size);
}

void free(void *p) {
	BT_kFree(p);
}


#ifdef BT_CONFIG_KERNEL_FREERTOS
void *pvPortMalloc() {
	return 0;
}

void vPortFree(void *p) {
	return;
}
#endif

