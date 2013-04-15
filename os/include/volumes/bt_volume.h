#ifndef _BT_VOLUME_H_
#define _BT_VOLUME_H_

BT_ERROR 	BT_EnumerateVolumes	(BT_HANDLE hBlockDevice);
BT_u32 		BT_VolumeRead		(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);
BT_u32 		BT_VolumeWrite		(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);

#endif
