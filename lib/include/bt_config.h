#ifndef _BT_CONFIG_H_
#define _BT_CONFIG_H_

#include "bt_arch_config.h"














/**
 *	Check for Architecture configuration sanity.
 **/
#ifndef BT_CONFIG_ARCH_LITTLE_ENDIAN
#ifndef BT_CONFIG_ARCH_BIG_ENDIAN
#error	"BT: No architecture endianess defined, modify the appropriate bt_arch_config.h file."
#endif
#endif


#endif
