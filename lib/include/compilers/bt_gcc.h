#ifndef _BT_GCC_H_
#define _BT_GCC_H_

#define BT_ATTRIBUTE_SECTION(name) __attribute__ ((section(name), used))
#define __BT_WEAK __attribute__((weak))


#define BT_CLZ(x)	__builtin_clz(x)













#endif
