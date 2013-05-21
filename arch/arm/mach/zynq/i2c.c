#include <bitthunder.h>
#include <collections/bt_fifo.h>
#include "i2c.h"
#include "slcr.h"

BT_DEF_MODULE_NAME			("Zynq-I2C")
BT_DEF_MODULE_DESCRIPTION	("I2C device driver for Zynq")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	ZYNQ_I2C_REGS 	   *pRegs;
};

