#ifndef _BT_LOADER_H_
#define _BT_LOADER_H_

typedef void (*BT_LOADER_SECTION_CB) (void *addr, void *data, BT_u32 flags, BT_u32 len);

typedef struct _BT_LOADER {
	const BT_i8 *name;
	BT_BOOL (*pfnCanDecode)(void *image_start, BT_u32 len);
	void   *(*pfnDecode)(void *image_start, BT_u32 len, BT_LOADER_SECTION_CB pfnLoad, BT_ERROR *pError);
} BT_LOADER;

#define BT_LOADER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.module.loaders") BT_LOADER

#endif
