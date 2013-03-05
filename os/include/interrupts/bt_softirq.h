/**
 *	BitThunder SoftIRQ implementation.
 *
 *
 **/

#ifndef _BT_SOFTIRQ_H_
#define _BT_SOFTIRQ_H_

typedef struct _BT_SOFTIRQ {
	BT_ERROR   (*pfnAction)	(struct _BT_SOFTIRQ *pSoftIRQ);
	void 	   *pData;
} BT_SOFTIRQ;


#endif
