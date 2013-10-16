/*
 * putc.h
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#ifndef PUTC_H_
#define PUTC_H_

#include <bt_config.h>

#define BT_stdin 	(BT_HANDLE)(-1)
#define BT_stdout 	(BT_HANDLE)(-1)

BT_ERROR BT_SetStdin(BT_HANDLE h);

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)

	#define BT_MULTIPLE_STDOUT_MAX	3

	BT_ERROR BT_AddStdout(BT_HANDLE h);

	void BT_RemoveStdout(BT_HANDLE h);

#else 

	BT_ERROR BT_SetStdout(BT_HANDLE h);

#endif

int bt_getc(void);
	
int bt_fgetc(void *stream);

void bt_putc(int c);
	
void bt_fputc(int c, void *stream);

#endif /* PUTC_H_ */
