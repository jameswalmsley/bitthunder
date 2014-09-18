#include <bitthunder.h>
#include <syscall/errno.h>
#include <errno.h>

long bt_sys_lseek(int fd, int offset, int whence) {
	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE hFile = BT_GetFileDescriptor(fd, &Error);
	if(!hFile) {
		errno = EBADF;
		return -1;
	}

	Error = BT_Seek(hFile, offset, whence);
	if(!Error) {
		return 0;	// POSIX require to return the new offset.
	}

	return -1;
}
