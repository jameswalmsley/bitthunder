#include <bitthunder.h>
#include <syscall/errno.h>
#include <errno.h>

/**
 *	@brief	POSIX read() call.
 *
 *	This function implements a fully POSIX compliant READ() call for the BitThunder ABI 0.9.2+.
 *	Note, that the userspace calls call directly into this from read() (which becomes a trap in the libc).
 *	However, when calling read() from kernel-space, the the libc simply directs to bt_sys_read().
 *
 **/
long bt_sys_read(int fd, void *ptr, size_t count) {

	long read = 0;

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hFile = BT_GetFileDescriptor(fd, &Error);
	if(!hFile) {
		errno = EBADF;
		return -1;
	}

	BT_s32 slRead = BT_Read(hFile, 0, count, ptr);

	// Convert return code to POSIX error code.

	return (long) slRead;
}
