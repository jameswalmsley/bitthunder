/*
 * putc.h
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#ifndef PUTC_H_
#define PUTC_H_

void bt_putc(int c, void *stream);
BT_HANDLE BT_GetStandardHandle();
BT_ERROR BT_SetStandardHandle(BT_HANDLE h);

#endif /* PUTC_H_ */
