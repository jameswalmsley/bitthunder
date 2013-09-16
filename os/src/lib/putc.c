/*
 * putc.c
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#include <bitthunder.h>

static BT_HANDLE g_hOut = NULL;

void bt_putc(int c, void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;
	BT_u8 val = (BT_u8) c;
	BT_Write(h, 0, 1, &val, NULL);
}

BT_HANDLE BT_GetStandardHandle() {
	return g_hOut;
}

BT_ERROR BT_SetStandardHandle(BT_HANDLE h) {
	g_hOut = h;
	return BT_ERR_NONE;
}
