/*
 * putc.h
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#ifndef PUTC_H_
#define PUTC_H_

#include <bt_config.h>

void bt_putc(int c, void *stream);

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)

	#define BT_MULTIPLE_STDOUT_MAX	3

	BT_HANDLE *BT_GetStandardHandles();

	BT_ERROR BT_AddStandardHandle(BT_HANDLE h);

	void BT_RemoveStandardHandle(BT_HANDLE h);

#else 

	BT_HANDLE BT_GetStandardHandle();

	BT_ERROR BT_SetStandardHandle(BT_HANDLE h);

#endif


#endif /* PUTC_H_ */
