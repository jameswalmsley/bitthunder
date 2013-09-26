/**
 *	Generic Executable Loader for BitThunder.
 *
 *	This loader provides a generic loader interface in which other
 *	executable formats can be supported.
 *
 **/

#include <bitthunder.h>
#include <loader/bt_loader.h>

extern const BT_LOADER * __bt_loaders_start;
extern const BT_LOADER * __bt_loaders_end;

void *bt_image_load(void *image_start, BT_u32 len, BT_LOADER_SECTION_CB pfnLoad, BT_ERROR *pError) {

	BT_u32 i;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_loaders_end - (BT_u32) &__bt_loaders_start);

	size /= sizeof(BT_LOADER);

	BT_LOADER *pLoader = (BT_LOADER *) &__bt_loaders_start;
	for(i = 0; i < size; i++) {
		if(pLoader->pfnCanDecode(image_start, len)) {
			return pLoader->pfnDecode(image_start, len, pfnLoad, pError);
		}
		pLoader++;
	}

	return NULL;
}
