/*
 * putc.c
 *
 *  Created on: Mar 16, 2013
 *      Author: wl
 */

#include <bitthunder.h>
#include <lib/putc.h>

static BT_HANDLE g_hStdin = NULL;

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)
	static BT_HANDLE g_hStdout[BT_MULTIPLE_STDOUT_MAX + 1] = { NULL };
#else
	static BT_HANDLE g_hStdout = NULL;
#endif

BT_ERROR BT_SetStdin(BT_HANDLE h) {
	g_hStdin = h;
	return BT_ERR_NONE;
}

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)

	BT_ERROR BT_AddStdout(BT_HANDLE h) {
		BT_s32 i;
		for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
			if(g_hStdout[i] == NULL) {
				g_hStdout[i] = h;
				break;
			}
		}
		if(i>=BT_MULTIPLE_STDOUT_MAX) return BT_ERR_GENERIC;
		return BT_ERR_NONE;
	}

	void BT_RemoveStdout(BT_HANDLE h) {
		BT_s32 i;
		for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
			if(g_hStdout[i] == h) {
				g_hStdout[i] = NULL;
				break;
			}
		}
	}

#else

	BT_ERROR BT_SetStdout(BT_HANDLE h) {
		g_hStdout = h;
		return BT_ERR_NONE;
	}

#endif

static int fgetc(void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;

	return BT_GetC(h, 0, NULL);
}

static void fputc(int c, void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;
	BT_u8 val = (BT_u8) c;

	if(c == '\n') {
		BT_u8 *s = (BT_u8 *) "\r\n";
		BT_Write(h, 0, 2, s, NULL);
	} else {
		BT_Write(h, 0, 1, &val, NULL);
	}
}

int bt_getc(void) {
	if(g_hStdin) return fgetc(g_hStdin);
	else return 0;
}

int bt_fgetc(void *stream) {
	if(!stream || (stream == BT_stdin)) {
		return bt_getc();
	} else {
		return fgetc(stream);
	}
}

#if (BT_CONFIG_LIB_PRINTF_SUPPORT_MULTIPLE_STDOUT)

	void bt_putc(int c) {
		int i;
		for(i=0;i<BT_MULTIPLE_STDOUT_MAX;i++) {
			if(g_hStdout[i]) {
				fputc(c, g_hStdout[i]);
			}
		}
	}

#else

	void bt_putc(int c) {
		if(g_hStdout) fputc(c, g_hStdout);
	}

#endif

void bt_fputc(int c, void *stream) {
	if(!stream || (stream == BT_stdout)) {
		bt_putc(c);
	} else {
		fputc(c, stream);
	}
}
