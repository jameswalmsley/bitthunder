#ifndef _ARM11CPU_H_
#define _ARM11CPU_H_

#define isb() 							__asm__ __volatile__ ("isb" : : : "memory")
#define dsb()	 						__asm__ __volatile__ ("dsb" : : : "memory")
#define dmb() 							__asm__ __volatile__ ("dmb" : : : "memory")

#define ARM_CP15_MAIN_ID				"p15, 0, %0,  c0,  c0, 0"
#define ARM_CP15_CACHE_TYPE				"p15, 0, %0,  c0,  c0, 1"
#define ARM_CP15_TCM_TYPE				"p15, 0, %0,  c0,  c0, 2"
#define ARM_CP15_TLB_TYPE				"p15, 0, %0,  c0,  c0, 3"
#define ARM_CP15_MULTI_PROC_AFFINITY	"p15, 0, %0,  c0,  c0, 5"

#define ARM_CP15_PROC_FEATURE_0			"p15, 0, %0,  c0,  c1, 0"
#define ARM_CP15_PROC_FEATURE_1			"p15, 0, %0,  c0,  c1, 1"
#define ARM_CP15_DEBUG_FEATURE_0		"p15, 0, %0,  c0,  c1, 2"
#define ARM_CP15_MEMORY_FEATURE_0		"p15, 0, %0,  c0,  c1, 4"
#define ARM_CP15_MEMORY_FEATURE_1		"p15, 0, %0,  c0,  c1, 5"
#define ARM_CP15_MEMORY_FEATURE_2		"p15, 0, %0,  c0,  c1, 6"
#define ARM_CP15_MEMORY_FEATURE_3		"p15, 0, %0,  c0,  c1, 7"

#define ARM_CP15_INST_FEATURE_0			"p15, 0, %0,  c0,  c2, 0"
#define ARM_CP15_INST_FEATURE_1			"p15, 0, %0,  c0,  c2, 1"
#define ARM_CP15_INST_FEATURE_2			"p15, 0, %0,  c0,  c2, 2"
#define ARM_CP15_INST_FEATURE_3			"p15, 0, %0,  c0,  c2, 3"
#define ARM_CP15_INST_FEATURE_4			"p15, 0, %0,  c0,  c2, 4"

#define ARM_CP15_CACHE_SIZE_ID			"p15, 1, %0,  c0,  c0, 0"
#define ARM_CP15_CACHE_LEVEL_ID			"p15, 1, %0,  c0,  c0, 1"
#define ARM_CP15_AUXILARY_ID			"p15, 1, %0,  c0,  c0, 7"

#define ARM_CP15_CACHE_SIZE_SEL			"p15, 2, %0,  c0,  c0, 0"

#define ARM_CP15_SYS_CONTROL			"p15, 0, %0,  c1,  c0, 0"
#define ARM_CP15_AUX_CONTROL			"p15, 0, %0,  c1,  c0, 1"
#define ARM_CP15_CP_ACCESS_CONTROL		"p15, 0, %0,  c1,  c0, 2"

#define ARM_CP15_SECURE_CONFIG			"p15, 0, %0,  c1,  c1, 0"
#define ARM_CP15_SECURE_DEBUG_ENABLE	"p15, 0, %0,  c1,  c1, 1"
#define ARM_CP15_NS_ACCESS_CONTROL		"p15, 0, %0,  c1,  c1, 2"
#define ARM_CP15_VIRTUAL_CONTROL		"p15, 0, %0,  c1,  c1, 3"

#define ARM_CP15_INVAL_DC_LINE_MVA_POC	"p15, 0, %0,  c7,  c6, 1"
#define ARM_CP15_INVAL_DC_LINE_SW		"p15, 0, %0,  c7,  c6, 2"
#define ARM_CP15_CLEAN_INVAL_DC_LINE_SW "p15, 0, %0,  c7, c14, 2"


#define ARM_CP15_CONTROL_TE_BIT			0x40000000
#define ARM_CP15_CONTROL_AFE_BIT		0x20000000
#define ARM_CP15_CONTROL_TRE_BIT		0x10000000
#define ARM_CP15_CONTROL_NMFI_BIT		0x08000000
#define ARM_CP15_CONTROL_EE_BIT			0x02000000
#define ARM_CP15_CONTROL_HA_BIT			0x00020000
#define ARM_CP15_CONTROL_RR_BIT			0x00004000
#define ARM_CP15_CONTROL_V_BIT			0x00002000
#define ARM_CP15_CONTROL_I_BIT			0x00001000
#define ARM_CP15_CONTROL_Z_BIT			0x00000800
#define ARM_CP15_CONTROL_SW_BIT			0x00000400
#define ARM_CP15_CONTROL_B_BIT			0x00000080
#define ARM_CP15_CONTROL_C_BIT			0x00000004
#define ARM_CP15_CONTROL_A_BIT			0x00000002
#define ARM_CP15_CONTROL_M_BIT			0x00000001

#define ARM_CP15_INVAL_IC_POU                  "p15, 0, %0,  c7,  c5, 0"
#define ARM_CP15_INVAL_IC_LINE_MVA_POU         "p15, 0, %0,  c7,  c5, 1"

#endif
