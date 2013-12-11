/**
 *	Generic Executable Loader for BitThunder.
 *
 *	This loader provides a generic loader interface in which other
 *	executable formats can be supported.
 *
 *	To add support for other executable formats simply compile another loader
 *	which defines a BT_LOADER structure using the BT_LOADER_DEF macro.
 *
 *	Later this loader mechanism will be extended to support re-location,
 *	and shared-library loading.
 *
 *	For now simple loading of statically compiled executables is supported.
 *
 *	The loader can also begin execution of images, it does this by creating
 *	a privileged process, in which the image is actually loaded and all
 *	memory mappings are created for the process.
 *
 *	At the end of loading, the process mode switched into user context on the cpu,
 *	and the process jumps to the executable entry.
 *
 *	This is possible if we place the executable thread code in a special
 *	page all on its own, where execution is possible from Kernel and User mode.
 *
 **/

#include <bitthunder.h>
#include <loader/bt_loader.h>

extern const BT_LOADER * __bt_loaders_start;
extern const BT_LOADER * __bt_loaders_end;

void *bt_image_load(void *image_start, BT_u32 len, BT_LOADER_SEGMENT_CB pfnLoad, void *pParam, BT_ERROR *pError) {

	BT_u32 i;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_loaders_end - (BT_u32) &__bt_loaders_start);

	size /= sizeof(BT_LOADER);

	BT_LOADER *pLoader = (BT_LOADER *) &__bt_loaders_start;
	for(i = 0; i < size; i++) {
		if(pLoader->pfnCanDecode(image_start, len)) {
			return pLoader->pfnDecode(image_start, len, pfnLoad, pParam, pError);
		}
		pLoader++;
	}

	return NULL;
}

static void executor_load_segment(BT_LOADER_SEGMENT *pSegment, void *pParam) {

	void *addr = pSegment->v_addr;
	BT_ERROR Error = bt_vm_allocate(curtask, &addr, pSegment->size, 0);
	memcpy(addr, pSegment->data, pSegment->data_len);
}

struct _BT_EXEC_LOADER_PARAMS {
	void   *image_start;
	BT_u32 	len;
	BT_BOOL bComplete;
};

typedef int (*entry_point) (int argc, char **argv);

/**
 *	This special function should be located in a page that is EXECUTABLE by both
 *	Kernel and user mode.
 *
 *	Upon switching to user mode, no kernel data will be available to
 *	the user code.
 *
 **/
static BT_ERROR exec_loader_thread(BT_HANDLE hThread, void *pParam) {

	BT_ERROR Error = BT_ERR_NONE;

	struct _BT_EXEC_LOADER_PARAMS *pParams = (struct _BT_EXEC_LOADER_PARAMS *) pParam;

	// Now we're running in the process's main thread, but in what is considered
	// to be the processes kernel mode.

	BT_HANDLE hProcess = BT_GetProcessHandle();
	void *entry = bt_image_load(pParams->image_start, pParams->len, executor_load_segment, hProcess, &Error);

	pParams->bComplete = BT_TRUE;

	BT_DCacheFlush();

	entry_point pfnEntry = (entry_point) entry;
	BT_u32 instruction = *((BT_u32 *) entry);

	pfnEntry(0, NULL);	// Currently pass process code no arguments, but we could provide a mechanism for a process to get its args without passing.

	// Either the process will quit using an exit() system call, or will return to here
	// In which case on return the process manager should cleanup all resources.

	// Its also possible that corrupt data was loaded, and we get an invalid instruction
	// fault. In this case the interrupt handler will kill the process, and yield to another
	// thread.

	return Error;
}

/**
 *	This is the default executor of in-memory executables,
 *	BT_Exec uses this after loading an executable image from the file-system.
 *
 *	Images are expected to be located on a 4byte boundary.
 *
 *	At this point, the image must be located within kernel addressable memory.
 **/
BT_ERROR BT_ExecImage(void *image_start, BT_u32 len, const BT_i8 *name) {
	BT_ERROR Error = BT_ERR_NONE;

	struct _BT_EXEC_LOADER_PARAMS *pParams = BT_kMalloc(sizeof(struct _BT_EXEC_LOADER_PARAMS));
	if(!pParams) {
		return BT_ERR_NO_MEMORY;
	}

	pParams->image_start 	= image_start;
	pParams->len 			= len;
	pParams->bComplete 		= BT_FALSE;

	BT_THREAD_CONFIG oConfig;

	oConfig.ulStackDepth 	= 0x100;	// 256 bytes initial stack for the decoder.
	oConfig.ulPriority 		= 0;		// Process can manage its own priority on startup.
	oConfig.ulFlags			= 0;
	oConfig.pParam			= pParams;

	BT_HANDLE hProcess = BT_CreateProcess(exec_loader_thread, name, &oConfig, &Error);
	while(!pParams->bComplete) {
		BT_ThreadSleep(100);
		continue;
	}

	BT_kFree(pParams);

	return Error;
}

BT_ERROR BT_ExecImageFile(const BT_i8 *szpPath) {
	// Executes an image from a file-system.
	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hFile = BT_Open(szpPath, BT_GetModeFlags("rb"), &Error);
	if(!hFile) {
		return Error;
	}

	BT_HANDLE hInode = BT_GetInode(szpPath, &Error);
	if(!hInode) {
		BT_CloseHandle(hFile);
		return Error;
	}

	BT_INODE oInode;
	BT_ReadInode(hInode, &oInode);

	BT_CloseHandle(hInode);

	void *image = BT_kMalloc(oInode.ullFilesize);
	BT_u32 read = BT_Read(hFile, 0, oInode.ullFilesize, image, &Error);

	return BT_ExecImage(image, read, szpPath);
}
