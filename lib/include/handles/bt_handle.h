/**
 *	@brief		Handle Manager
 *
 *	Provides Kernel-level Handle management for the BitThunder operating system.
 *
 *	@file		bt_handle.h
 *	@author 	James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	HANDLE
 *
 *	@copyright	(c)2012 James Walmsley
 **/
/**
 *	@defgroup	HANDLE	Handle Manager
 *	@brief		Kernel-level API for managing BT_HANDLE objects.
 *
 *	The Handle Manager is used by almost all other modules to create/attach BT_HANDLE
 *	objects. A HANDLE is a primitive allowing any resource within the system to be
 *	safely referenced in a type-safe way.
 *
 *	The HANDLE (BT_HANDLE) is the most important primitive object (data-structure)
 *	within the BitThunder operating system. It is used to manage all resources within
 *	the OS (with the exception of memory), and allow intelligent reporting and cleanup.
 **/
#ifndef _BT_HANDLE_H_
#define _BT_HANDLE_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"
#include "collections/bt_list.h"

/**
 *	This is the primary object of BitThunder.
 *	Every HANDLE must be derived/based on this structure.
 **/
typedef struct _BT_HANDLE_HEADER {
	struct bt_list_head		item;
	const BT_IF_HANDLE	   *pIf;				///< Pointer to the handle interface.
	BT_u32					ulReferenceCount;	///< The number of times the handle is used.
	//BT_u32 					ulFlags;			///<
} BT_HANDLE_HEADER;

#define BT_HANDLE_TYPE(x)	(((BT_HANDLE_HEADER *) (x))->pIf->eType)

/**
 *	@brief	Attach a BT_HANDLE to a process.
 *
 *	This function can be used to attach a manually created handle to a running process.
 *	If you created a BT_HANDLE using the @ref BT_CreateHandle() function then you don't
 *	need to invoke this function.
 *
 *	This function also initialises the HANDLE header to use the HANDLE interface that
 *	required to make the handle work.
 *
 *	@param[in] 	hProcess	Handle of the process that the handle should be attached to.
 *	@param[in]	pIf			The Handle interface that your HANDLE (h) will use.
 *	@param[in]	h			The Handle that is to be attached.
 *
 *	@return	BT_ERR_NONE on success.
 *
 **/
BT_ERROR BT_AttachHandle(BT_HANDLE hProcess, const BT_IF_HANDLE *pIf, BT_HANDLE h);

/**
 *	@brief 	Create and Attach a new BT_HANDLE to the currently running process.
 *
 *	This function is really an allocator, and will create a HANDLE object and initialise
 *	it with the required HANDLE interface.
 *
 *	@param[in]		hProcess		Process handle to attach to, or simply NULL for current process.
 *	@param[in]		pIf				The Handle interface that the HANDLE will use.
 *	@param[in]		ulHandleMemory	The total size of the handle object to be created in bytes.
 *	@param[out,opt]	pError			A BT_ERROR code to provide information about any failure.
 *
 *	@return A valid, initialised and attached BT_HANDLE object ready for use.
 *	@return NULL on error, check the value of pError.
 *
 **/
BT_HANDLE BT_CreateHandleAttached(BT_HANDLE hProcess, const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError);

#define BT_CreateHandle(interface, size, error_pointer)	BT_CreateHandleAttached(NULL, interface, size, error_pointer)

/**
 *	@brief	Destroy a BT_HANDLE object, and detach it from any associated process.
 *
 *	This function is used to destroy a BT_HANDLE cleanly. It will correctly detach the handle
 *	from any associated processes, and if applicable notify any HANDLE notifiers.
 *
 *	@note 	This function does not invoke the HANDLE cleanup mechanism!
 *	@note	In most cases you should use @ref BT_CloseHandle()
 *
 *	@param[in]	h	Handle to be destroyed.
 *
 *	@return	BT_ERR_NONE on success.
 **/
BT_ERROR BT_DestroyHandle(BT_HANDLE h);

/**
 *	@brief	Gracefully cleanup, detach and destroy a BT_HANDLE object.
 *
 *	This function acts as the primary destructor for any HANDLE (objects) within the
 *	BitThunder operating system.
 *
 *	All HANDLE notifiers are notified.
 *	All cleanup routines for the handle are invoked.
 *	The HANDLE is detached.
 *	The HANDLE is destroyed (if applicable).
 *
 *	@param[in]	h	Handle to be closed.
 *
 *	@return	BT_ERR_NONE on sucess.
 *
 **/
BT_ERROR BT_CloseHandle(BT_HANDLE h);

/**
 *	@brief	Increment a HANDLE's reference count.
 *
 *
 **/
BT_ERROR BT_RefHandle(BT_HANDLE h);

 /**
  *	@brief	Decrement a HANDLE's reference count.
  *
  **/
BT_ERROR BT_UnrefHandle(BT_HANDLE h);

#endif
