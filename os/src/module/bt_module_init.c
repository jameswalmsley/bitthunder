/**
 *	Kernel Module initialisation routines.
 *
 **/

#include <bt_types.h>
#include <bt_error.h>
#include <module/bt_module_init.h>

extern const BT_MODULE_INIT * __bt_module_init_start;
extern const BT_MODULE_INIT * __bt_module_init_end;

BT_ERROR BT_InitialiseKernelModules(BT_HANDLE hLogger) {
	BT_ERROR Error;
	BT_u32 i;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_module_init_end - (BT_u32) &__bt_module_init_start);

	size /= sizeof(BT_MODULE_INIT);

	for(i = 0; i < size; i++) {
		BT_MODULE_INIT *pInit = (BT_MODULE_INIT *) &__bt_module_init_start;
		pInit += i;

		if(pInit->pfnInit) {
			Error = pInit->pfnInit();
			if(Error) {
				// Here we should try to log a failure!
			}
		}
	}

	return BT_ERR_NONE;
}

