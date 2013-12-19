//*****************************************************************************
//
// Include OS functionality.
//
//*****************************************************************************

/* ------------------------ System architecture includes ----------------------------- */
#include "net/lwip/arch/sys_arch.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "lwip/opt.h"

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

/* Very crude mechanism used to determine if the critical section handling
functions are being called from an interrupt context or not.  This relies on
the interrupt handler setting this variable manually. */
static void *g_pMutex = NULL;


/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates a new mailbox
 * Inputs:
 *      int size                -- Size of elements in the mailbox
 * Outputs:
 *      sys_mbox_t              -- Handle to new mailbox
 *---------------------------------------------------------------------------*/
err_t sys_mbox_new( sys_mbox_t *pxMailBox, int iSize ) {
	err_t xReturn = ERR_MEM;
	BT_ERROR Error = BT_ERR_NONE;

	*pxMailBox = BT_CreateQueue( (BT_u32)iSize, sizeof( void * ), &Error);

	if( *pxMailBox != NULL ) {
		xReturn = ERR_OK;
		SYS_STATS_INC_USED( mbox );
	}
	return xReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a mailbox. If there are messages still present in the
 *      mailbox when the mailbox is deallocated, it is an indication of a
 *      programming error in lwIP and the developer should be notified.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 * Outputs:
 *      sys_mbox_t              -- Handle to new mailbox
 *---------------------------------------------------------------------------*/
void sys_mbox_free( sys_mbox_t *pxMailBox ) {
	BT_u32 ulMessagesWaiting;

	ulMessagesWaiting = BT_QueueMessagesWaiting( *pxMailBox );

	#if SYS_STATS
	{
		if( ulMessagesWaiting != 0UL ) {
			SYS_STATS_INC( mbox.err );
		}

		SYS_STATS_DEC( mbox.used );
	}
	#endif /* SYS_STATS */

	BT_CloseHandle( *pxMailBox );
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_post
 *---------------------------------------------------------------------------*
 * Description:
 *      Post the "msg" to the mailbox.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void *data              -- Pointer to data to post
 *---------------------------------------------------------------------------*/
void sys_mbox_post( sys_mbox_t *pxMailBox, void *pxMessageToPost ) {
	BT_QueueSendToBack( *pxMailBox, &pxMessageToPost, 0 );
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_mbox_trypost
 *---------------------------------------------------------------------------*
 * Description:
 *      Try to post the "msg" to the mailbox.  Returns immediately with
 *      error if cannot.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void *msg               -- Pointer to data to post
 * Outputs:
 *      err_t                   -- ERR_OK if message posted, else ERR_MEM
 *                                  if not.
 *---------------------------------------------------------------------------*/
err_t sys_mbox_trypost( sys_mbox_t *pxMailBox, void *pxMessageToPost ) {
	BT_ERROR xReturn;

	xReturn = BT_QueueSend( *pxMailBox, &pxMessageToPost, 0 );

	if( xReturn == BT_TRUE ) {
		xReturn = ERR_OK;
	}
	else {
		/* The queue was already full. */
		xReturn = ERR_MEM;
		SYS_STATS_INC( mbox.err );
	}

	return xReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_mbox_fetch
 *---------------------------------------------------------------------------*
 * Description:
 *      Blocks the thread until a message arrives in the mailbox, but does
 *      not block the thread longer than "timeout" milliseconds (similar to
 *      the sys_arch_sem_wait() function). The "msg" argument is a result
 *      parameter that is set by the function (i.e., by doing "*msg =
 *      ptr"). The "msg" parameter maybe NULL to indicate that the message
 *      should be dropped.
 *
 *      The return values are the same as for the sys_arch_sem_wait() function:
 *      Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
 *      timeout.
 *
 *      Note that a function with a similar name, sys_mbox_fetch(), is
 *      implemented by lwIP.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void **msg              -- Pointer to pointer to msg received
 *      u32_t timeout           -- Number of milliseconds until timeout
 * Outputs:
 *      u32_t                   -- SYS_ARCH_TIMEOUT if timeout, else number
 *                                  of milliseconds until received.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_mbox_fetch( sys_mbox_t *pxMailBox, void **ppvBuffer, u32_t ulTimeOut ) {
	void *pvDummy;
	BT_TICK ulStartTime, ulEndTime, ulElapsed;
	BT_u32 ulReturn;

	ulStartTime = BT_GetKernelTick();

	if( NULL == ppvBuffer ) {
		ppvBuffer = &pvDummy;
	}


	if( ulTimeOut != 0UL ) {
		if( BT_TRUE == BT_QueueReceive( *pxMailBox, (void *) ppvBuffer, ulTimeOut ) ) {
			ulEndTime = BT_GetKernelTick();
			ulElapsed = ( ulEndTime - ulStartTime );

			ulReturn = ulElapsed;
		}
		else {
			/* Timed out. */
			*ppvBuffer = NULL;
			ulReturn = SYS_ARCH_TIMEOUT;
		}
	}
	else {
		while( BT_TRUE != BT_QueueReceive( *pxMailBox, (void *) ppvBuffer, 1000 ) );
		ulEndTime = BT_GetKernelTick();
		ulElapsed = ( ulEndTime - ulStartTime );

		if( ulElapsed == 0UL ) {
			ulElapsed = 1UL;
		}

		ulReturn = ulElapsed;
	}

	return ulReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_mbox_tryfetch
 *---------------------------------------------------------------------------*
 * Description:
 *      Similar to sys_arch_mbox_fetch, but if message is not ready
 *      immediately, we'll return with SYS_MBOX_EMPTY.  On success, 0 is
 *      returned.
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      void **msg              -- Pointer to pointer to msg received
 * Outputs:
 *      u32_t                   -- SYS_MBOX_EMPTY if no messages.  Otherwise,
 *                                  return ERR_OK.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_mbox_tryfetch( sys_mbox_t *pxMailBox, void **ppvBuffer ) {
	void *pvDummy;
	unsigned long ulReturn;
	long lResult;

	if( ppvBuffer== NULL ) {
		ppvBuffer = &pvDummy;
	}

	lResult = BT_QueueReceive( *pxMailBox, (void *) ppvBuffer, 0UL );

	if( lResult == BT_TRUE ) {
		ulReturn = ERR_OK;
	}
	else {
		ulReturn = SYS_MBOX_EMPTY;
	}

	return ulReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Creates and returns a new semaphore. The "ucCount" argument specifies
 *      the initial state of the semaphore.
 *      NOTE: Currently this routine only creates counts of 1 or 0
 * Inputs:
 *      sys_mbox_t mbox         -- Handle of mailbox
 *      u8_t ucCount              -- Initial ucCount of semaphore (1 or 0)
 * Outputs:
 *      sys_sem_t               -- Created semaphore or 0 if could not create.
 *---------------------------------------------------------------------------*/
err_t sys_sem_new( sys_sem_t *pxSemaphore, u8_t ucCount ) {
	err_t xReturn = ERR_MEM;

	*pxSemaphore = BT_kMutexCreate();

	if( *pxSemaphore != NULL ) {
		if( ucCount == 0U ) {
			BT_kMutexPend( *pxSemaphore, 0 );
		}
		xReturn = ERR_OK;
		SYS_STATS_INC_USED( sem );
	}
	else {
		SYS_STATS_INC( sem.err );
	}

	return xReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_sem_wait
 *---------------------------------------------------------------------------*
 * Description:
 *      Blocks the thread while waiting for the semaphore to be
 *      signaled. If the "timeout" argument is non-zero, the thread should
 *      only be blocked for the specified time (measured in
 *      milliseconds).
 *
 *      If the timeout argument is non-zero, the return value is the number of
 *      milliseconds spent waiting for the semaphore to be signaled. If the
 *      semaphore wasn't signaled within the specified time, the return value is
 *      SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
 *      (i.e., it was already signaled), the function may return zero.
 *
 *      Notice that lwIP implements a function with a similar name,
 *      sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to wait on
 *      u32_t timeout           -- Number of milliseconds until timeout
 * Outputs:
 *      u32_t                   -- Time elapsed or SYS_ARCH_TIMEOUT.
 *---------------------------------------------------------------------------*/
u32_t sys_arch_sem_wait( sys_sem_t *pxSemaphore, u32_t ulTimeout ) {
	BT_u32 ulStartTime, ulEndTime, ulElapsed;
	BT_u32 ulReturn;

	ulStartTime = BT_GetKernelTick();

	if( ulTimeout != 0UL ) {
		if(BT_kMutexPend( *pxSemaphore, ulTimeout) == BT_TRUE ) {
			ulEndTime = BT_GetKernelTick();
			ulElapsed = (ulEndTime - ulStartTime);
			ulReturn = ulElapsed;
		}
		else {
			ulReturn = SYS_ARCH_TIMEOUT;
		}
	}
	else {
		BT_kMutexPend( *pxSemaphore, 0);
		ulEndTime = BT_GetKernelTick();
		ulElapsed = ( ulEndTime - ulStartTime );

		if( ulElapsed == 0UL ) {
			ulElapsed = 1UL;
		}

		ulReturn = ulElapsed;
	}

	return ulReturn;
}

err_t sys_mutex_new( sys_mutex_t *Mutex ) {
	err_t xReturn = ERR_MEM;

	*Mutex = BT_kMutexCreate();

	if( *Mutex != NULL ) {
		xReturn = ERR_OK;
		SYS_STATS_INC_USED( mutex );
	}
	else {
		SYS_STATS_INC( mutex.err );
	}

	return xReturn;
}

/** Lock a mutex
 * @param mutex the mutex to lock */
void sys_mutex_lock( sys_mutex_t *Mutex ) {
	BT_kMutexPend( *Mutex, 0 );
}

/** Unlock a mutex
 * @param mutex the mutex to unlock */
void sys_mutex_unlock(sys_mutex_t *Mutex ) {
	BT_kMutexRelease( *Mutex );
}


/** Delete a semaphore
 * @param mutex the mutex to delete */
void sys_mutex_free( sys_mutex_t *Mutex ) {
	SYS_STATS_DEC( mutex.used );
	BT_kMutexDestroy( *Mutex );
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_signal
 *---------------------------------------------------------------------------*
 * Description:
 *      Signals (releases) a semaphore
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to signal
 *---------------------------------------------------------------------------*/
void sys_sem_signal( sys_sem_t *pxSemaphore ) {

	BT_kMutexRelease( *pxSemaphore );
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_sem_free
 *---------------------------------------------------------------------------*
 * Description:
 *      Deallocates a semaphore
 * Inputs:
 *      sys_sem_t sem           -- Semaphore to free
 *---------------------------------------------------------------------------*/
void sys_sem_free( sys_sem_t *pxSemaphore ) {
	SYS_STATS_DEC(sem.used);
	BT_kMutexDestroy( *pxSemaphore );
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_init
 *---------------------------------------------------------------------------*
 * Description:
 *      Initialize sys arch
 *---------------------------------------------------------------------------*/
void sys_init(void) {
	g_pMutex = BT_kRecursiveMutexCreate();
}

u32_t sys_now(void) {
	return BT_GetKernelTick();
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_thread_new
 *---------------------------------------------------------------------------*
 * Description:
 *      Starts a new thread with priority "prio" that will begin its
 *      execution in the function "thread()". The "arg" argument will be
 *      passed as an argument to the thread() function. The id of the new
 *      thread is returned. Both the id and the priority are system
 *      dependent.
 * Inputs:
 *      char *name              -- Name of thread
 *      void (* thread)(void *arg) -- Pointer to function to run.
 *      void *arg               -- Argument passed into function
 *      int stacksize           -- Required stack amount in bytes
 *      int prio                -- Thread priority
 * Outputs:
 *      sys_thread_t            -- Pointer to per-thread timeouts.
 *---------------------------------------------------------------------------*/

struct thread_params {
	void *pParam;
	void (*pxThread) (void *pvParameters);
};

static BT_ERROR lwip_thread_startup(BT_HANDLE hThread, void *pParam) {

	struct thread_params *params = (struct thread_params *) pParam;
	params->pxThread(params->pParam);
	return BT_ERR_NONE;
}

sys_thread_t sys_thread_new( const char *pcName, void( *pxThread )( void *pvParameters ), void *pvArg, int iStackSize, int iPriority ) {
	BT_HANDLE hThread;
	BT_ERROR Error = BT_ERR_NONE;
	sys_thread_t xReturn;

	BT_THREAD_CONFIG oThreadConfig;
	struct thread_params *params = BT_kMalloc(sizeof(*params));

	oThreadConfig.ulStackDepth = iStackSize;
	oThreadConfig.ulPriority = iPriority;
	oThreadConfig.ulFlags = 0;
	oThreadConfig.pParam = params;

	params->pParam = pvArg;
	params->pxThread = pxThread;

	hThread = BT_CreateThread(lwip_thread_startup, &oThreadConfig, &Error);

	if( Error == BT_ERR_NONE ) {
		xReturn = hThread;
	} else {
		xReturn = NULL;
	}

	return xReturn;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_protect
 *---------------------------------------------------------------------------*
 * Description:
 *      This optional function does a "fast" critical region protection and
 *      returns the previous protection level. This function is only called
 *      during very short critical regions. An embedded system which supports
 *      ISR-based drivers might want to implement this function by disabling
 *      interrupts. Task-based systems might want to implement this by using
 *      a mutex or disabling tasking. This function should support recursive
 *      calls from the same task or interrupt. In other words,
 *      sys_arch_protect() could be called while already protected. In
 *      that case the return value indicates that it is already protected.
 *
 *      sys_arch_protect() is only required if your port is supporting an
 *      operating system.
 * Outputs:
 *      sys_prot_t              -- Previous protection level (not used here)
 *---------------------------------------------------------------------------*/
sys_prot_t sys_arch_protect( void ) {
	BT_kMutexPendRecursive(g_pMutex, 0);

	return ( sys_prot_t ) 1;
}

/*---------------------------------------------------------------------------*
 * Routine:  sys_arch_unprotect
 *---------------------------------------------------------------------------*
 * Description:
 *      This optional function does a "fast" set of critical region
 *      protection to the value specified by pval. See the documentation for
 *      sys_arch_protect() for more information. This function is only
 *      required if your port is supporting an operating system.
 * Inputs:
 *      sys_prot_t              -- Previous protection level (not used here)
 *---------------------------------------------------------------------------*/
void sys_arch_unprotect( sys_prot_t xValue ) {
	(void) xValue;
	BT_kMutexReleaseRecursive(g_pMutex);
}

/*
 * Prints an assertion messages and aborts execution.
 */
void sys_assert( const char *pcMessage ) {
	(void) pcMessage;

	for (;;) {
	}
}
/*-------------------------------------------------------------------------*
 * End of File:  sys_arch.c
 *-------------------------------------------------------------------------*/
