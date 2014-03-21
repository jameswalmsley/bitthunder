#ifndef _BT_HANDLES_H_
#define _BT_HANDLES_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"
#include "collections/bt_list.h"

/**
 *	This is the primary object of BitThunder.
 *	Every HANDLE must be derived/based on this structure.
 **/
typedef struct _BT_HANDLE_HEADER {
	struct bt_list_head		list;
	const BT_IF_HANDLE	   *pIf;				///< Pointer to the handle interface.
	BT_u32					ulClaimedMemory;	///< Amount of memory claimed by this handle.
} BT_HANDLE_HEADER;

#define BT_HANDLE_TYPE(x)	(x->h.pIf->eType)

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
 *	@brief 	Create and Attach a new BT_HANDLE to currently running process.
 *
 *	This function is really an allocator, and will create a HANDLE object and initialise
 *	it with the required HANDLE interface.
 *
 *	@param[in]		pIf				The Handle interface that the HANDLE will use.
 *	@param[in]		ulHandleMemory	The total size of the handle object to be created in bytes.
 *	@param[out,opt]	pError			A BT_ERROR code to provide information about any failure.
 *
 *	@return A valid, initialised and attached BT_HANDLE object ready for use.
 *	@return NULL on error, check the value of pError.
 *
 **/
BT_HANDLE BT_CreateHandle(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError);

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

#endif
