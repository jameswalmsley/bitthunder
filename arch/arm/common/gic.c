#include <bitthunder.h>
#include <arch/common/gic.h>
#include <interrupts/bt_interrupts.h>
#include <bt_module.h>

BT_DEF_MODULE_NAME				("Arm GIC")
BT_DEF_MODULE_DESCRIPTION		("Provides a complete interrupt controller interface for BitThunder")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

#ifndef BT_CONFIG_ARCH_ARM_GIC_BASE
#error "Error: BT_CONFIG_ARCH_ARM_GIC_BASE not defined in HAL."
#endif

#ifndef BT_CONFIG_ARCH_ARM_GIC_DIST_BASE
#error "Error: BT_CONFIG_ARCH_ARM_GIC_DIST_BASE not defined in HAL."
#endif

#define GIC_BASE 		BT_CONFIG_ARCH_ARM_GIC_BASE
#define GIC_DIST_BASE	BT_CONFIG_ARCH_ARM_GIC_DIST_BASE

#define GICC	((GICC_REGS *) GIC_BASE)
#define GICD	((GICD_REGS *) GIC_DIST_BASE)

static BT_ERROR gic_initialise(BT_u32 ulTotalIRQs) {
	//GICC_REGS *pCRegs = GICC;
	//GICD_REGS *pDRegs = GICD;

	return BT_ERR_NONE;
}

static BT_ERROR gic_cleanup() {
	return BT_ERR_NONE;
}

static BT_ERROR gic_register(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR gic_unregister(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR gic_setpriority(BT_u32 ulIRQ, BT_u32 ulPriority) {
	return BT_ERR_NONE;
}

static BT_u32 gic_getpriority(BT_u32 ulIRQ, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR gic_enable(BT_u32 ulIRQ) {
	return BT_ERR_NONE;
}

static BT_ERROR gic_disable(BT_u32 ulIRQ) {
	return BT_ERR_NONE;
}

static BT_u32 gic_get_total_irqs(BT_ERROR *pError) {
	return 0;
}

static const BT_INTERRUPT_CONTROLLER oDeviceInterface = {
	.oModuleInfo 		= BT_MODULE_DEF_INFO,
	.pfnInitialise 		= gic_initialise,
	.pfnCleanup			= gic_cleanup,
	.pfnRegister		= gic_register,
	.pfnUnregister		= gic_unregister,
	.pfnSetPriority		= gic_setpriority,
	.pfnGetPriority		= gic_getpriority,
	.pfnEnable			= gic_enable,
	.pfnDisable			= gic_disable,
	.pfnSetAffinity		= NULL,						///< An option interface, GIC could implement this.
	.pfnGetTotalIRQs	= gic_get_total_irqs,
};
