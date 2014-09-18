#include <bitthunder.h>
#include <syscall/errno.h>
#include <errno.h>

struct flag_mask {
	BT_u32 flag;
	BT_u32 mask;
};

#define FLAGS_DEFAULT BT_FS_MODE_READ			// In c flags = 0 mean read only.

static const struct flag_mask libc_to_bt_flags[] = {
	{BT_FS_MODE_WRITE, 						~(BT_FS_MODE_READ)},		// O_RDONLY
	{BT_FS_MODE_READ | BT_FS_MODE_WRITE, 	0},							// O_RDWR
	{0, 0},
	{BT_FS_MODE_APPEND, 					0},							// O_APPEND
	{0, 0},																// _FMARK
	{0, 0},																// _FDEFER
	{0, 0},																// _FASYNC
	{0, 0},																// _FSHLOCK
	{0, 0},																// _FEXLOCK
	{BT_FS_MODE_CREATE,						0},							// O_CREAT
	{BT_FS_MODE_TRUNCATE, 					0},							// O_TRUNC
};

static BT_u32 convert_flags(int flags) {
	BT_u32 i;
	BT_u32 bt_flags = FLAGS_DEFAULT;

	for(i = 0; i < sizeof(libc_to_bt_flags)/sizeof(struct flag_mask); i++) {
		if((1 << i) & flags) {
			if(libc_to_bt_flags[i].mask) {
				bt_flags &= libc_to_bt_flags[i].mask;
			}

			bt_flags |= libc_to_bt_flags[i].flag;
		}
	}

	return bt_flags;
}

long bt_sys_open(const char *pathname, int flags, int mode) {

	BT_ERROR Error = BT_ERR_NONE;
	int fd = (int) BT_AllocFileDescriptor();
	if(fd < 0) {
		errno = EMFILE;
		return -1;			// Process has reached its open FD limit.
	}

	BT_u32 bt_flags = convert_flags(flags);

	BT_HANDLE hFile = BT_Open(pathname, bt_flags, &Error);
	if(!hFile) {
		BT_FreeFileDescriptor(fd);
		errno = ENOENT;
		return -1;
	}

	BT_SetFileDescriptor(fd, hFile);

	return (long) fd;
}
