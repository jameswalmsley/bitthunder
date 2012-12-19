/**
 *	BlueThunder - Device Manager
 *
 **/

#ifndef _BT_DEVMAN_H_
#define _BT_DEVMAN_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"


typedef const BT_IF_HANDLE 	   *(*BT_MODULE_GETIF_FN)(void);
typedef BT_ERROR				(*BT_MODULE_INIT_FN)(void);

typedef struct {
  const BT_s8 	   	   *szpDeviceName;
  BT_MODULE_INIT_FN		pfnModuleEntry;
  const BT_IF_HANDLE   *pIf;
} BT_MODULE_ENTRY_DESCRIPTOR;


#define BT_MODULE_ENTRY(entry) 		   BT_ATTRIBUTE_SECTION(".bt.module.entries") 	\
  static const BT_MODULE_ENTRY_DESCRIPTOR *pModuleEntry = &entry;


BT_HANDLE BT_DeviceOpen(BT_u32 ulId, const BT_s8 *szpDevicePath, BT_ERROR *pError);
BT_MACHINE_DESCRIPTION *BT_GetMachineDescription(BT_ERROR *pError);

#endif
