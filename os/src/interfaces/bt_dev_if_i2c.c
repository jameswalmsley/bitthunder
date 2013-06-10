/**
 *	I2C Configuration API.
 *
 *
 **/
#include <bitthunder.h>
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	BT_HANDLE 		 hBusMutex;
};

static BT_BOOL isI2CHandle(BT_HANDLE hI2C) {
	if(!hI2C || !BT_IF_DEVICE(hI2C) || (BT_IF_DEVICE_TYPE(hI2C) != BT_DEV_IF_T_I2C)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

BT_ERROR BT_I2C_LockBus(BT_HANDLE hI2C) {
	return BT_PendMutex(hI2C->hBusMutex, 0);
}

BT_ERROR BT_I2C_UnlockBus(BT_HANDLE hI2C) {
	return BT_ReleaseMutex(hI2C->hBusMutex);
}

BT_u32 BT_I2C_Transfer(BT_HANDLE hI2C, BT_I2C_MESSAGE *pMessages, BT_u32 ulMessages, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	if(pError) {
		*pError = Error;
	}

	if(!isI2CHandle(hI2C)) {
		Error = BT_ERR_INVALID_HANDLE;
		goto err_out;
	}

	const BT_DEV_IF_I2C *pOps = BT_IF_I2C_OPS(hI2C);
	if(!pOps->pfnMasterTransfer) {
		Error = BT_ERR_GENERIC;		// This I2C controller doesn't support standard I2C messages.
		goto err_out;
	}

	BT_u32 RetVal;

	BT_I2C_LockBus(hI2C);
	{
		RetVal = pOps->pfnMasterTransfer(hI2C, pMessages, ulMessages, pError);
	}
	BT_I2C_UnlockBus(hI2C);

	return RetVal;

err_out:
	if(pError) {
		*pError = Error;
	}

	return 0;
}


BT_u32 BT_I2C_MasterSend(BT_I2C_CLIENT *pClient, const BT_u8 *pucSource, BT_u32 ulLength, BT_ERROR *pError) {

	BT_I2C_MESSAGE oMessage;
	oMessage.addr 	= pClient->addr;
	oMessage.flags 	= pClient->flags & BT_I2C_M_TEN;
	oMessage.len	= ulLength;
	oMessage.buf	= (BT_u8 *) pucSource;

	BT_u32 RetVal = BT_I2C_Transfer(pClient->hBus, &oMessage, 1, pError);

	return (RetVal == 1) ? ulLength : RetVal;
}

BT_u32 BT_I2C_MasterReceive(BT_I2C_CLIENT *pClient, BT_u8 *pucDest, BT_u32 ulLength, BT_ERROR *pError) {

	BT_I2C_MESSAGE oMessage;
	oMessage.addr 	= pClient->addr;
	oMessage.flags 	= pClient->flags & BT_I2C_M_TEN;
	oMessage.flags |= BT_I2C_M_RD;
	oMessage.len 	= ulLength;
	oMessage.buf	= pucDest;

	BT_u32 RetVal = BT_I2C_Transfer(pClient->hBus, &oMessage, 1, pError);

	return (RetVal == 1) ? ulLength : RetVal;
}

BT_I2C_CLIENT *BT_I2C_NewDevice(BT_HANDLE hI2C, BT_I2C_BOARD_INFO *pInfo, BT_ERROR *pError) {
	return NULL;
}
