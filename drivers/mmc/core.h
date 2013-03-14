#ifndef _CORE_H_
#define _CORE_H_

//struct _MMC_HOST;
typedef struct _MMC_HOST MMC_HOST;

typedef struct _MMC_COMMAND {
	BT_u32		opcode;
	BT_u32		arg;
	BT_u32		response[4];
	BT_BOOL		bCRC;
	BT_u32		ulResponseType;
} MMC_COMMAND;

typedef enum _BT_MMC_CARD_EVENT {
	BT_MMC_CARD_DETECTED,
	BT_MMC_CARD_REMOVED,
} BT_MMC_CARD_EVENT;

typedef void (*BT_MMC_CARD_EVENTRECEIVER)(MMC_HOST *pHost, BT_MMC_CARD_EVENT eEvent, BT_BOOL bInterruptContext);

typedef struct _BT_MMC_OPS {
	BT_u32 		ulCapabilites1;			///< Primary Capability flags.

	BT_ERROR 	(*pfnEventSubscribe)	(BT_HANDLE hSDIO, BT_MMC_CARD_EVENTRECEIVER pfnReceiver, MMC_HOST *pHost);
	BT_BOOL	 	(*pfnIsCardPresent)		(BT_HANDLE hSDIO, BT_ERROR *pError);
	BT_ERROR	(*pfnInitialise)		(BT_HANDLE hSDIO);

	BT_ERROR 	(*pfnRequest)			(BT_HANDLE hSDIO, MMC_COMMAND *pCommand);
} BT_MMC_OPS;

BT_ERROR BT_RegisterSDHostController(BT_HANDLE hHost, const BT_MMC_OPS *pOps);


#endif
