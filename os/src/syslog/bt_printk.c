/**
 * BitThunder - Syslog Printk.
 **/

#include <bt_error.h>
#include <lib/printf.h>
#include <lib/putc.h>

BT_ERROR BT_kPrint(const char *format, ... ) {
	va_list ap;

	bt_printf("[%5d.%06d] : ", BT_GetKernelTick(), 0);

	va_start(ap, format);
	bt_kvprintf(format, bt_putc, (void *) BT_GetStandardHandle(), 10, ap);
	va_end(ap);

	return BT_ERR_NONE;
}
