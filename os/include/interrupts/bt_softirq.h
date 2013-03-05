/**
 *	BitThunder SoftIRQ implementation.
 *
 *
 **/

#ifndef _BT_SOFTIRQ_H_
#define _BT_SOFTIRQ_H_

#ifndef BT_CONFIG_SOFTIRQ_MAX
#define BT_CONFIG_SOFTIRQ_MAX	32
#endif

#ifndef BT_CONFIG_SOFTIRQ_PRIORITY
#define BT_CONFIG_SOFTIRQ_PRIORITY	0
#endif

/*
 *	SoftIRQ numbers.
 */

#define BT_SOFTIRQ_HI		0		///< The highest priority soft IRQ.
#define BT_SOFTIRQ_TASKLET	1		///< The default soft IRQ where a tasklet will run.

struct _BT_SOFTIRQ;

typedef void (*BT_SOFTIRQ_HANDLER)(struct _BT_SOFTIRQ *pSoftIRQ);

typedef struct _BT_SOFTIRQ {
	BT_SOFTIRQ_HANDLER 	pfnHandler;
	void 	   		   *pData;
} BT_SOFTIRQ;

BT_ERROR BT_OpenSoftIRQ			(BT_u32 ulSoftIRQ, BT_SOFTIRQ_HANDLER pfnHandler, void *pData);
BT_ERROR BT_RaiseSoftIRQ		(BT_u32 ulSoftIRQ);
BT_ERROR BT_RaiseSoftIRQFromISR	(BT_u32 ulSoftIRQ);

#endif
