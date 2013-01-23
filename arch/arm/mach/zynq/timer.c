/**
 *	Provides the Xilinx system timer for BitThunder.
 **/

#include <bitthunder.h>
#include "slcr.h"			///< Provides access to the SLCR registers.

BT_DEF_MODULE_NAME			("ZYNQ-TIMER")
BT_DEF_MODULE_DESCRIPTION	("ZYNQ Timers kernel driver, also providing kernel tick")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_HANDLE timer_open(BT_u32 nDeviceID, BT_ERROR *pError) {
	return NULL;
}

static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_getinputclock(BT_HANDLE hTimer, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR timer_start(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_HANDLE timer_register_callback(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	return NULL;
}

static BT_ERROR timer_unregister_callback(BT_HANDLE hTimer, BT_HANDLE hCallback) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_get_prescaler(BT_HANDLE hTimer, BT_ERROR *pError) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_set_prescaler(BT_HANDLE hTimer, BT_u32 ulPrescaler) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_get_period_count(BT_HANDLE hTimer, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR timer_set_period_count(BT_HANDLE hTimer, BT_u32 ulValue) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_enable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_disable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_getvalue(BT_HANDLE hTimer, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR timer_setvalue(BT_HANDLE hTimer, BT_u32 ulValue) {
	return BT_ERR_UNIMPLEMENTED;
}


/*
 *
 *
 *
 */

static const BT_DEV_IF_TIMER oTimerDeviceInterface= {
	timer_getinputclock,
	timer_start,
	timer_stop,
	timer_enable_interrupt,
	timer_disable_interrupt,
	NULL,
	timer_register_callback,
	timer_unregister_callback,
	timer_get_prescaler,
	timer_set_prescaler,
	timer_get_period_count,
	timer_set_period_count,
	timer_enable_reload,
	timer_disable_reload,
	timer_getvalue,
	timer_setvalue,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oTimerDeviceInterface,
};

const BT_IF_DEVICE BT_ZYNQ_TIMER_oDeviceInterface = {
	1,
	timer_open,
	NULL,					/// No power interface for system timer.
	BT_DEV_IF_T_TIMER,
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oTimerDeviceInterface,
	},
};

/*
 *	Because this will provide the primary timer, we need to export its symbol.
 */
BT_EXPORT_SYMBOL(BT_ZYNQ_TIMER_oDeviceInterface)


static const BT_UN_IFS oDevIF = {
	(BT_HANDLE_INTERFACE) &BT_ZYNQ_TIMER_oDeviceInterface,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_NAME,												///< Provides the standard module name in ROM.
	BT_MODULE_DESCRIPTION,
	BT_MODULE_AUTHOR,
	BT_MODULE_EMAIL,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_ZYNQ_TIMER_oDeviceInterface,
	},													///< Pointer to a Device interface if its a device.
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	timer_cleanup,												///< Handle's cleanup routine.
};

static const BT_MODULE_ENTRY_DESCRIPTOR entryDescriptor = {
	(BT_s8 *) "timer",
	NULL,					///< No driver init function required!
	&oHandleInterface,
};

BT_MODULE_ENTRY(entryDescriptor);
