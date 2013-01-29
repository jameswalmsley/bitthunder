#ifndef _BITTHUNDER_H_
#define _BITTHUNDER_H_

#define BT_VERSION_MAJOR	0
#define BT_VERSION_MINOR	3
#define BT_VERSION_REVISION	0

#define BT_VERSION_NAME		"Blinking Tortoise"

#include "bt_config.h"
#include "bt_types.h"
#include "bt_error.h"
#include "bt_struct.h"
#include "mm/bt_mm.h"
#include "handles/bt_handle.h"
#include "machines/bt_machines.h"

#ifdef BT_CONFIG_OS
#include "bt_os.h"
#endif

#endif
