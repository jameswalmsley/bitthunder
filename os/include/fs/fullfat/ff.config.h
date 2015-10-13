#ifndef _FF_BT_CONFIG_H_
#define _FF_BT_CONFIG_H_

#include <bt_types.h>
#include <mm/bt_heap.h>
#include <bt_bsp_config.h>

#define sprintf bt_sprintf

#define FF_BITTHUNDER

#ifdef BT_CONFIG_BIG_ENDIAN
#define ffconfigBYTE_ORDER  pdFREERTOS_BIG_ENDIAN
#endif

#ifdef BT_CONFIG_LITTLE_ENDIAN
#define ffconfigBYTE_ORDER  pdFREERTOS_LITTLE_ENDIAN
#endif

#define	ffconfigHAS_CWD						0

#define FF_INLINE inline
#define portINLINE  FF_INLINE

#define BaseType_t  BT_s32
#define UBaseType_t BT_u32
#define TickType_t  BT_TICK

#define pdFALSE     0
#define pdTRUE      1
//typedef BT_u32 time_t;

#define pdMS_TO_TICKS(x)    (((TickType_t) x * BT_CONFIG_KERNEL_TICK_RATE) / (TickType_t) 1000)

#define FF_PRINTF   BT_kPrint

#ifdef BT_CONFIG_FS_FULLFAT_ALLOC_DEFAULT
#define FF_ALLOC_DEFAULT
#endif

#ifdef BT_CONFIG_FS_FULLFAT_ALLOC_DOUBLE
#define FF_ALLOC_DOUBLE
#endif

#ifdef BT_CONFIG_FS_FULLFAT_LFN_SUPPORT
#define ffconfigLFN_SUPPORT                 1
#endif

#define ffconfigTIME_SUPPORT                1

#define ffconfigWRITE_FREE_COUNT            1

#define ffconfigFSINFO_TRUSTED              1

#define FF_DRIVER_BUSY_SLEEP BT_CONFIG_FS_FULLFAT_DRIVER_BUSY_SLEEP

#define ffconfigMALLOC(aSize)	            BT_kMalloc(aSize)
#define ffconfigFREE(aPtr)		            BT_kFree(aPtr)

#define ffconfigFINDAPI_ALLOW_WILDCARDS     1

#define ffconfigSHORTNAME_CASE              1

#define ffconfigDEBUG                       1

#define FF_NOSTRCASECMP                     0

#define ffconfigOPTIMISE_UNALIGNED_ACCESS   1

#define ffconfigWRITE_BOTH_FATS             1

#define	ffconfigNAMES_ON_HEAP				BT_CONFIG_FS_FULLFAT_LFN_ON_HEAP

#define configASSERT(x)

#endif
