#ifndef _BT_MODULE_INIT_
#define _BT_MODULE_INIT_

typedef struct _BT_MODULE_INIT {
	const BT_i8 *name;
	BT_ERROR (*pfnInit) (void);
} BT_MODULE_INIT;

#define BT_MODULE_INIT_DEF 	static const BT_ATTRIBUTE_SECTION(".bt.module.init") BT_MODULE_INIT

#endif
