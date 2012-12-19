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

#ifndef BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS
#error "Error: BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS not defined in HAL."
#endif

#define GIC_BASE 		BT_CONFIG_ARCH_ARM_GIC_BASE
#define GIC_DIST_BASE	BT_CONFIG_ARCH_ARM_GIC_DIST_BASE


#define GICC	((GICC_REGS *) GIC_BASE)
#define GICD	((GICD_REGS *) GIC_DIST_BASE)


BT_ERROR gic_initialise() {
	//GICC_REGS *pCRegs = GICC;
	//GICD_REGS *pDRegs = GICD;



	return BT_ERR_NONE;
}


const BT_INTC BT_ARM_GIC_oInterface = {
	.oModuleInfo 		= BT_MODULE_DEF_INFO,
	.ulTotalInterrupts 	= BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS,
	.pfnInitialise 		= gic_initialise,
};
