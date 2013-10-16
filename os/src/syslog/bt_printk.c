/**
 * BitThunder - Syslog Printk.
 **/

#include <bt_config.h>
#include <bt_error.h>
#include <lib/printf.h>
#include <lib/putc.h>
#include <timers/bt_timers.h>

#ifndef BT_CONFIG_SYSLOG_REMOVE_PRINTK
BT_ERROR BT_kPrint(const char *format, ... ) {
	va_list ap;

#ifndef BT_CONFIG_KERNEL_NONE
	BT_u64 gt 	= BT_GetGlobalTimer();
	BT_u32 rate = BT_GetGlobalTimerRate();

	bt_printf("[%5d.%06d] : ", (BT_u32) (gt / (BT_u64)rate), (BT_u32) ((1000000 * (gt % (BT_u64)rate)) / rate ));
#endif

	va_start(ap, format);
	bt_kvprintf(format, bt_fputc, BT_stdout, 10, ap);
	va_end(ap);

#ifdef BT_CONFIG_SYSLOG_LINE_ENDINGS_CR
	bt_printf("\n");
#endif
#ifdef BT_CONFIG_SYSLOG_LINE_ENDINGS_LF
	bt_printf("\r");
#endif
#ifdef BT_CONFIG_SYSLOG_LINE_ENDINGS_CRLF
	bt_printf("\n\r");
#endif
#ifdef BT_CONFIG_SYSLOG_LINE_ENDINGS_LFCR
	bt_printf("\r\n");
#endif
	return BT_ERR_NONE;
}
#endif
