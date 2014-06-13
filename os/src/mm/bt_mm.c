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
BT_EXPORT_SYMBOL(BT_Calloc);


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

void *realloc(void *p, int size) {
	return BT_kRealloc(p, size);
}


#ifdef BT_CONFIG_KERNEL_FREERTOS
void *pvPortMalloc(int size) {
	return BT_kMalloc(size);
}

void vPortFree(void *p) {
	BT_kFree(p);
}
#endif
