#include <bitthunder.h>
#include <lib/putc.h>

BT_ERROR BT_SetStdin(BT_HANDLE h) {
	BT_SetFileDescriptor(0, h);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_SetStdin);

BT_HANDLE BT_GetStdin() {
	BT_ERROR Error;
	return BT_GetFileDescriptor(0, &Error);
}
BT_EXPORT_SYMBOL(BT_GetStdin);

BT_ERROR BT_SetStdout(BT_HANDLE h) {
	BT_SetFileDescriptor(1, h);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_SetStdout);

BT_HANDLE BT_GetStdout() {
	BT_ERROR Error;
	return BT_GetFileDescriptor(1, &Error);
}
BT_EXPORT_SYMBOL(BT_GetStdout);

BT_ERROR BT_SetStderr(BT_HANDLE h) {
	BT_SetFileDescriptor(2, h);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_SetStderr);

BT_HANDLE BT_GetStderr() {
	BT_ERROR Error;
	return BT_GetFileDescriptor(2, &Error);
}
BT_EXPORT_SYMBOL(BT_GetStderr);

static int fgetc(void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;

	return BT_GetC(h, 0);
}

static void fputc(int c, void *stream) {
	BT_HANDLE h = (BT_HANDLE) stream;
	BT_u8 val = (BT_u8) c;

	if(c == '\n') {
		BT_u8 *s = (BT_u8 *) "\r\n";
		BT_Write(h, 0, 2, s);
	} else {
		BT_Write(h, 0, 1, &val);
	}
}

int bt_getc(void) {
	BT_HANDLE h = BT_GetStdin();
	if(h) {
		return fgetc(h);
	}

	return 0;
}
BT_EXPORT_SYMBOL(bt_getc);

int bt_fgetc(void *stream) {
	return fgetc(stream);
}
BT_EXPORT_SYMBOL(bt_fgetc);

void bt_putc(int c) {
	BT_HANDLE h = BT_GetStdout();
	if(h) {
		fputc(c, h);
	}
}
BT_EXPORT_SYMBOL(bt_putc);

void bt_fputc(int c, void *stream) {
	fputc(c, stream);
}
BT_EXPORT_SYMBOL(bt_fputc);
