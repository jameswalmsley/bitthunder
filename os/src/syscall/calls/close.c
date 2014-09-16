#include <bitthunder.h>

long bt_sys_close(int fd) {
	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hFile = BT_GetFileDescriptor(fd, &Error);
	BT_SetFileDescriptor(fd, NULL);
	BT_CloseHandle(hFile);
	BT_FreeFileDescriptor(fd);

	return 0;
}
