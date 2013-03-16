/**
 * BitThunder printf replacement and macro override.
 */

#ifndef _PRINTF_H_
#define _PRINTF_H_

#include <stdarg.h>

int 	bt_kvprintf	(char const *fmt, void (*func)(int, void*), void *arg, int radix, va_list ap);
void 	bt_printf	(const char *fmt, ...);

#endif
