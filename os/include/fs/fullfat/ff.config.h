#ifndef _FF_BT_CONFIG_H_
#define _FF_BT_CONFIG_H_

#include <bt_bsp_config.h>

#ifdef BT_CONFIG_BIG_ENDIAN
#define FF_BIG_ENDIAN
#endif

#define FF_INLINE static inline

#ifdef BT_CONFIG_FS_FULLFAT_ALLOC_DEFAULT
#define FF_ALLOC_DEFAULT
#endif

#ifdef BT_CONFIG_FS_FULLFAT_ALLOC_DOUBLE
#define FF_ALLOC_DOUBLE
#endif

#ifdef BT_CONFIG_FS_FULLFAT_LFN_SUPPORT
#define FF_LFN_SUPPORT
#endif

#define FF_DRIVER_BUSY_SLEEP BT_CONFIG_FS_FULLFAT_DRIVER_BUSY_SLEEP

#define FF_MALLOC(aSize)	malloc(aSize)
#define FF_FREE(aPtr)		free(aPtr)

#define FF_FINDAPI_ALLOW_WILDCARDS

#define FF_DEBUG

#define FF_NOSTRCASECMP

#endif
