#ifndef _FF_BT_CONFIG_H_
#define _FF_BT_CONFIG_H_

#include <bt_types.h>
#include <mm/bt_heap.h>
#include <bt_bsp_config.h>

#ifdef BT_CONFIG_BIG_ENDIAN
#define FF_BIG_ENDIAN
#endif

#ifdef BT_CONFIG_LITTLE_ENDIAN
#define FF_LITTLE_ENDIAN
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

#define FF_TIME_SUPPORT

#define FF_WRITE_FREE_COUNT

#define FF_FSINFO_TRUSTED

#define FF_DRIVER_BUSY_SLEEP BT_CONFIG_FS_FULLFAT_DRIVER_BUSY_SLEEP

#define FF_MALLOC(aSize)	BT_kMalloc(aSize)
#define FF_FREE(aPtr)		BT_kFree(aPtr)

#define FF_FINDAPI_ALLOW_WILDCARDS

#define FF_SHORTNAME_CASE

#define FF_DEBUG

#define FF_NOSTRCASECMP

#define FF_OPTIMISE_UNALIGNED_ACCESS

#endif
