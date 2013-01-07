/**
 *	BlueThunder - Device Manager
 *
 **/

#ifndef _BT_DEVMAN_H_
#define _BT_DEVMAN_H_

#include "bt_types.h"
#include "bt_resource.h"
#include "bt_integrated_device.h"
#include "bt_integrated_driver.h"
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
BT_INTEGRATED_DEVICE *BT_GetIntergratedDeviceByName(const BT_i8 *szpName);
BT_INTEGRATED_DRIVER *BT_GetIntegratedDriverByName(const BT_i8 *szpName);

BT_ERROR BT_ProbeIntegratedDevices(BT_HANDLE hLogDevice);

#endif
