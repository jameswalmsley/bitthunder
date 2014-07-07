#ifndef _BT_PROCESS_H_
#define _BT_PROCESS_H_

#include <collections/bt_list.h>

#define BT_PATH_MAX		2600

struct bt_task {
	BT_i8				name[BT_CONFIG_MAX_PROCESS_NAME+1];
	BT_u32				pid;
	struct bt_task 	   *parent;
	struct bt_vm_map   *map;
	struct bt_list_head threads;
	struct bt_list_head handles;
	BT_u64 				ullRunTimeCounter;
	BT_u32				flags;
    #define 			BT_PFD_BITMAP	0xFF000000	///< Bitmap of allocated primary bitmaps.
    #define 			BT_FD_BITMAP	0x00800000	///< If fd_tbl was allocated.
	BT_u32				free_fd;					///< Number of the last known free FD.
	BT_HANDLE			fds[8];
#ifdef BT_CONFIG_PROCESS_CWD
	BT_i8			    cwd[BT_PATH_MAX];
#endif
};

struct bt_process_time {
	BT_u64 		ullRunTimeCounter;
	BT_u32 		ulRunTimePercent;
	BT_HANDLE 	hProcess;
	char 	   *name;
};

#define curtask 	curthread->task

BT_ERROR BT_StartScheduler();

/**
 *	@brief	Create a new process within Kernel-space.
 *
 *	There is no specific api to create a user-space process, simply the user-space
 *	code should be loaded by a start-routine that configures the memory map and loads
 *	the executable code.
 *
 *	Once loaded, the routine switches the cpu into unprivileged mode and jumps to
 *	the untrusted code's entry point.
 *
 *	@pfnStartRoutine	[IN]	Pointer to executable code where this process will start.
 *	@szpName			[IN]	Name of the process as seen by the kernel.
 * 	@pConfig			[IN]	Pointer to BT_THREAD_CONFIG parameters for starting this process.
 *	@pError				[OUT]	A BT_ERROR code in case of errors.
 *
 *	@return Handle to process created.
 **/
BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

/**
 *	@brief	Get handle of the currently executing process.
 *
 *	@return NULL if not a process context.
 *	@return	BT_HANDLE of the current process.
 **/
BT_HANDLE BT_GetProcessHandle(void);
BT_HANDLE BT_GetKernelProcessHandle(void);
BT_HANDLE BT_GetParentProcessHandle(void);

struct bt_task *BT_GetProcessTask(BT_HANDLE hProces);

BT_ERROR BT_GetProcessTime(struct bt_process_time *time, BT_u32 i);
BT_u32 BT_GetTotalProcesses();

BT_s32 BT_AllocFileDescriptor();
BT_ERROR BT_FreeFileDescriptor(BT_s32 fd);
BT_ERROR BT_SetFileDescriptor(BT_u32 i, BT_HANDLE h);
BT_HANDLE BT_GetFileDescriptor(BT_u32 i, BT_ERROR *pError);
BT_ERROR BT_SetProcessFileDescriptor(BT_HANDLE hProcess, BT_u32 i, BT_HANDLE h);
BT_HANDLE BT_GetProcessFileDescriptor(BT_HANDLE hProcess, BT_u32 i, BT_ERROR *pError);

BT_ERROR bt_process_init();

#endif
