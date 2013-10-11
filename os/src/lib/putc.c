/*
 * putc.c
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#include <bitthunder.h>
#include <lib/putc.h>

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)
	static BT_HANDLE g_hOut[BT_MULTIPLE_STDOUT_MAX + 1] = { NULL };
#else
	static BT_HANDLE g_hOut = NULL;
#endif

static void putc(int c, void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;
	BT_u8 val = (BT_u8) c;

	if(c == '\n') {
		BT_u8 *s = (BT_u8 *) "\r\n";
		BT_Write(h, 0, 2, s, NULL);
	} else {
		BT_Write(h, 0, 1, &val, NULL);
	}
}

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)

	void bt_putc(int c, void *stream) {
		if(stream) {
			putc(c, stream);
		} else {
			BT_HANDLE *phStdout = BT_GetStandardHandles();
			int i;
			for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
				if(phStdout[i]) {
					putc(c, phStdout[i]);
				}
			}
		}
	}

	BT_HANDLE *BT_GetStandardHandles() {
		return g_hOut;
	}

	BT_ERROR BT_AddStandardHandle(BT_HANDLE h) {
		BT_s32 i;
		for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
			if(g_hOut[i] == NULL) {
				g_hOut[i] = h;
				break;
			}
		}
		if(i>=BT_MULTIPLE_STDOUT_MAX) return BT_ERR_GENERIC;
		return BT_ERR_NONE;
	}

	void BT_RemoveStandardHandle(BT_HANDLE h) {
		BT_s32 i;
		for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
			if(g_hOut[i] == h) {
				g_hOut[i] = NULL;
				break;
			}
		}
	}

#else

	void bt_putc(int c, void *stream) {
		if(stream) putc(c, stream);
		else putc(c, g_hOut);
	}

	BT_HANDLE BT_GetStandardHandle() {
		return g_hOut;
	}

	BT_ERROR BT_SetStandardHandle(BT_HANDLE h) {
		g_hOut = h;
		return BT_ERR_NONE;
	}

#endif
