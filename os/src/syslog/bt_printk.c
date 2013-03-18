/**
 * BitThunder - Syslog Printk.
 **/

#include <bt_error.h>
#include <lib/printf.h>
#include <lib/putc.h>

BT_ERROR BT_kPrint(const char *format, ... ) {
	va_list ap;

	BT_TICK oTicks = BT_GetKernelTick();
	bt_printf("[%5d.%03d] : ", oTicks / 1000, oTicks % 1000);

	va_start(ap, format);
	bt_kvprintf(format, bt_putc, (void *) BT_GetStandardHandle(), 10, ap);
	va_end(ap);

	bt_printf("\n");

	return BT_ERR_NONE;
}
