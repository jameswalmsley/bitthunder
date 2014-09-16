#include <bitthunder.h>
#include <syscall/errno.h>

long bt_sys_write(int fd, const void *ptr, size_t count) {

	long written = 0;

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hFile = BT_GetFileDescriptor(fd, &Error);
	if(!hFile) {
		return -EBADF;
	}

	BT_s32 slWritten = BT_Write(hFile, 0, count, &Error);

	// Convert return code to POSIX error code.

	return (long) slWritten;
}
