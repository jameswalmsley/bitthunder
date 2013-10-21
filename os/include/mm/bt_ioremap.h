#ifndef _BT_IOREMAP_H_
#define _BT_IOREMAP_H_

#include <bt_config.h>
#include <bt_types.h>

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
void *bt_ioremap(void *phys_addr, BT_u32 size);
void bt_iounmap(volatile void *iomem);
#else
#define bt_ioremap(x, y)	x
#define bt_iounmap(x)
#endif

#endif
