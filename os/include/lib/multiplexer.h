#ifndef _BT_MULTIPLEXER_H_
#define _BT_MULTIPLEXER_H_

BT_HANDLE 	BT_CreateMux(BT_u32 ulFlags, BT_ERROR *pError);
BT_ERROR 	BT_MuxOpen(BT_HANDLE hMux, const BT_i8 *path);
BT_ERROR 	BT_MuxAttach(BT_HANDLE hMux, BT_HANDLE h);


#endif
