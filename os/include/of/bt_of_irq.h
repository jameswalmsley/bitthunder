#ifndef _BT_OF_IRQ_H_
#define _BT_OF_IRQ_H_


#define BT_OF_MAX_IRQ_SPEC	4

struct bt_of_irq {
	struct bt_device_node *controller;
	BT_u32 size;
	BT_u32 specifier[BT_OF_MAX_IRQ_SPEC];
};

BT_u32 bt_of_irq_count(struct bt_device_node *dev);
BT_u32 bt_of_irq_to_resource_table(struct bt_device_node *device, BT_RESOURCE *r, BT_u32 nr_irqs);


#endif
