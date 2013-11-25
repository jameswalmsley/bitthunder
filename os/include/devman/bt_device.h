#ifndef _BT_DEVICE_H_
#define _BT_DEVICE_H_

#include "bt_types.h"
#include "bt_resource.h"
#include <collections/bt_list.h>
#include <fs/bt_devfs.h>

typedef enum {
	#define BT_DEVICE_TYPE_CODE_MASK	0x000000FF
	#define BT_DEVICE_TYPE_OF_FLAG		0x80000000
	BT_DEVICE_INTEGRATED= 0,	///< Platform/Integrated device type.
	BT_DEVICE_BUS,
	BT_DEVICE_I2C,				///< I2C device type.
} BT_DEVICE_TYPE;

/**
 *	This structure literally describes a device.
 **/
typedef struct _BT_DEVICE {
	BT_u32 		eType;
	union {
		struct {
			const BT_i8		   *name;
			BT_u32				ulTotalResources;
			const BT_RESOURCE  *pResources;
		};
	};
} BT_DEVICE;

/**
 *	This is the device node structure used to link all devices in the system into
 *	a tree.
 *
 *	This tree is created using the FDT if open-firmware support is enabled.
 **/
struct bt_device_property {
	struct bt_list_head 	item;					///< Forms an item in a list of properties on the bt_device_node.
	const char 		   	   *name;					///< Property name (from the device tree).
	BT_u32		 			length;					///< Length of property data in bytes.
	void	   		   	   *value;					///< Pointer to the properties data.
	BT_u32					flags;					///< Property flags.
};

struct bt_device_node {
	struct bt_list_head		item;					///< Forms an item of children of a root or parent node.
	struct bt_list_head		allitem;	   			///< Forms an item on a linear list, in device tree order.
	struct bt_list_head 	children;				///< List of the device's children.
	struct bt_device_node  *parent;					///< Pointer to parent, e.g. integrated device pointer to simple-bus.
	const BT_i8 		   *name;					///< Device's name.
	const BT_i8			   *full_name;				///< Device's name with full device tree path.
	const BT_i8			   *type;					///< device_type field.
	BT_u32					phandle;				///< Provides O(n) lookup time of a node from the fdt.
	BT_u32					node_offset;			///< Provides O(1) lookup time of a node from the fdt.
	struct bt_list_head		properties;				///< List of node/device properties.
	BT_u32 					ulFlags;				///< Flags to specify probed/populated etc.
    #define 				BT_DEVICE_POPULATED 	0x00000001	///< Device was populated, e.g. integrated resources formed.
    #define 				BT_DEVICE_PROBED		0x00000002	///< Device was probed already.
	BT_DEVICE 				dev;					///< Device structure as passed to the driver probe. Drivers can cast out to device_node but only if CONFIG_OF is enabled.
	struct bt_devfs_node	devfs_node;				///< A devfs_node to be used by drivers needing a devfs_node, but only when CONFIG_OF is supported.
};

#define BT_DEVICE_DEF	static const BT_ATTRIBUTE_SECTION(".bt.devices") BT_DEVICE

BT_u32 				BT_GetTotalDevicesByType(BT_DEVICE_TYPE eType);
const BT_DEVICE    *BT_GetDeviceByType		(BT_DEVICE_TYPE eType, BT_u32 ulIndex);
const BT_RESOURCE  *BT_GetDeviceResource	(const BT_DEVICE *pDevice, BT_u32 ulType, BT_u32 ulNum);

#endif
