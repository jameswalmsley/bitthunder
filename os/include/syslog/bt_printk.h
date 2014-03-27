/*
 * bt_syslog.h
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#ifndef _BT_PRINTK_H_
#define _BT_PRINTK_H_

#ifndef BT_CONFIG_SYSLOG_REMOVE_PRINTK
BT_ERROR BT_kPrint(const char *format, ... );
#define BT_kDebug(format, args ... )	BT_kPrint("%s : %s(): " format, BT_MODULE_NAME, __func__, ##args )
#else
#define BT_kPrint(...)
#define BT_kDebug(...)
#endif

#endif
