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
#else
#define BT_kPrint(...)
#endif

#endif
