/*
 * putc.h
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#ifndef PUTC_H_
#define PUTC_H_

#include <bt_config.h>

BT_ERROR 	BT_SetStdin(BT_HANDLE h);
BT_HANDLE 	BT_GetStdin();
BT_ERROR 	BT_SetStdout(BT_HANDLE h);
BT_HANDLE	BT_GetStdout();
BT_ERROR 	BT_SetStderr(BT_HANDLE h);
BT_HANDLE 	BT_GetStderr();

int bt_getc(void);
int bt_fgetc(void *stream);
void bt_putc(int c);
void bt_fputc(int c, void *stream);

#endif /* PUTC_H_ */
