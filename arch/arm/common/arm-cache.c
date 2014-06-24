#include <bitthunder.h>
#include "pl310.h"
#include "arm11cpu.h"

static volatile PL310_REGS *g_pregs = NULL;

/* CP15 operations */
#define wrcp(rn, v)	__asm__ __volatile__(\
			 "mcr " rn "\n"\
			 : : "r" (v)\
			);

#define rdcp(rn)	({unsigned int reg; \
			 __asm__ __volatile__(\
			   "mrc " rn "\n"\
			   : "=r" (reg)\
			 );\
			 reg;\
			 })


static BT_ERROR BT_L2CacheFlush() {
	register BT_u32 L2CCReg;
	volatile PL310_REGS *pRegs = g_pregs;
	/* Flush the caches */

	pRegs->reg7_clean_inv_way = 0x0000FFFF;

	/* Wait for the flush to complete */
	do {
		L2CCReg = pRegs->reg7_cache_sync;
	} while (L2CCReg != 0);

	/* synchronize the processor */
	dsb();

	return BT_ERR_NONE;
}

BT_ERROR BT_L2CacheInvalidateRange(void *addr, BT_u32 len) {
	volatile PL310_REGS *pRegs = (PL310_REGS *) g_pregs;
	const BT_u32 cacheline = 32;
	BT_u32 end;

	if(len) {
		end = (BT_u32) addr + len;
		BT_u32 adr = (BT_u32) addr & ~(cacheline - 1);

		while(adr < end) {
		    pRegs->reg7_inv_pa = adr;
			adr += cacheline;
		}
	}

	while(pRegs->reg7_cache_sync);

	dsb();

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_L2CacheInvalidateRange);

BT_ERROR BT_L2CacheInvalidateLine(void *addr) {
	volatile PL310_REGS *pRegs = (PL310_REGS *) g_pregs;
	pRegs->reg7_inv_pa = (BT_u32) addr;
	dsb();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_L2CacheInvalidateLine);

static BT_ERROR BT_L2CacheInvalidate() {
	register unsigned int L2CCReg;
	volatile PL310_REGS *pRegs = (PL310_REGS *) g_pregs;

	/* Invalidate the caches */
	pRegs->reg7_inv_way = 0x0000FFFF;

	/* Wait for the invalidate to complete */
	do {
		L2CCReg = pRegs->reg7_cache_sync;
	} while (L2CCReg != 0);

	/* synchronize the processor */
	dsb();

	return BT_ERR_NONE;
}

static BT_ERROR BT_L2CacheEnable() {
	register BT_u32 L2CCReg,CtrlReg;
	volatile PL310_REGS *pRegs = g_pregs;

	L2CCReg = pRegs->reg1_control;

	/* only enable if L2CC is currently disabled */
	if ((L2CCReg & 0x01) == 0) {
		/* set up the way size and latencies */
		L2CCReg = pRegs->reg1_aux_control;
		L2CCReg &= 0xFFF1FFFF;
		L2CCReg |= 0x72360000;	// Enable all pre-fetching.
		pRegs->reg1_aux_control = L2CCReg;

		pRegs->reg1_tag_ram_control = 0x00000111; // TAG RAM latency.
		pRegs->reg1_data_ram_control = 0x00000121; // DATA ram latency;


		/* Clear the pending interrupts */
		L2CCReg = pRegs->reg2_int_raw_status;
		pRegs->reg2_int_clear = L2CCReg;

		/* Enable the L2CC */
		L2CCReg = pRegs->reg1_control;
		pRegs->reg1_control = (L2CCReg | 0x01);

        /* synchronize the processor */
	    dsb();

        CtrlReg = rdcp(ARM_CP15_SYS_CONTROL);

        /* enable the Data cache */
        CtrlReg |= (ARM_CP15_CONTROL_C_BIT);

        wrcp(ARM_CP15_SYS_CONTROL, CtrlReg);

        /* synchronize the processor */
        dsb();
    }

	return BT_ERR_NONE;
}

static BT_ERROR BT_L2CacheDisable() {

    register BT_u32 L2CCReg,CtrlReg;
	volatile PL310_REGS *pRegs = (PL310_REGS *) g_pregs;

	L2CCReg = pRegs->reg1_control;

    if(L2CCReg & 0x1) {
	    CtrlReg = rdcp(ARM_CP15_SYS_CONTROL);

       	/* disable the Data cache */
	    CtrlReg &= ~(ARM_CP15_CONTROL_C_BIT);

    	wrcp(ARM_CP15_SYS_CONTROL, CtrlReg);

        /* synchronize the processor */
    	dsb();

        /* Clean and Invalidate L2 Cache */
        BT_L2CacheFlush();

	    /* Disable the L2CC */
    	L2CCReg = pRegs->reg1_control;
	    pRegs->reg1_control = (L2CCReg & (~0x01));

    	/* enable the Data cache */
	    CtrlReg |= (ARM_CP15_CONTROL_C_BIT);

    	wrcp(ARM_CP15_SYS_CONTROL, CtrlReg);

        /* synchronize the processor */
    	dsb();
    }

	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheInvalidateLine(void *addr) {
	wrcp(ARM_CP15_CACHE_SIZE_SEL, 0);
	wrcp(ARM_CP15_INVAL_DC_LINE_MVA_POC, ((BT_u32) addr & (~0x1F)));
	dsb();
	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheInvalidate() {
	register BT_u32 csid_reg, C7Reg;
	BT_u32 CacheSize, LineSize, NumWays;
	BT_u32 Way, WayIndex, Set, SetIndex, NumSet;

	/* Select cache level 0 and D cache in CSSR */
	wrcp(ARM_CP15_CACHE_SIZE_SEL, 0);
	isb();
	csid_reg = rdcp(ARM_CP15_CACHE_SIZE_ID);

	/* Determine Cache Size */
	CacheSize = (csid_reg >> 13) & 0x1FF;
	CacheSize +=1;
	CacheSize *=128;    /* to get number of bytes */

	/* Number of Ways */
	NumWays = (csid_reg & 0x3ff) >> 3;
	NumWays += 1;

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (csid_reg & 0x07) + 4;

	NumSet = CacheSize/NumWays;
	NumSet /= (1 << LineSize);

	Way = 0UL;
	Set = 0UL;

	/* Invalidate all the cachelines */
	for (WayIndex =0; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set;
			/* Invalidate by Set/Way */
			__asm__ __volatile__("mcr " \
			ARM_CP15_INVAL_DC_LINE_SW :: "r" (C7Reg));
			Set += (1 << LineSize);
		}
		Way += 0x40000000;
	}

	/* Wait for L1 invalidate to complete */
	dsb();

	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheFlushLine(void *addr) {
	wrcp(ARM_CP15_CACHE_SIZE_SEL, 0);
	wrcp(ARM_CP15_INVAL_DC_LINE_MVA_POC, ((BT_u32) addr & (~0x1F)));
	dsb();
	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheFlush() {
	register BT_u32 CsidReg, C7Reg;
	BT_u32 CacheSize, LineSize, NumWays;
	BT_u32 Way, WayIndex, Set, SetIndex, NumSet;

	/* Select cache level 0 and D cache in CSSR */
	wrcp(ARM_CP15_CACHE_SIZE_SEL, 0);
	isb();

	CsidReg = rdcp(ARM_CP15_CACHE_SIZE_ID);

	/* Determine Cache Size */

	CacheSize = (CsidReg >> 13) & 0x1FF;
	CacheSize +=1;
	CacheSize *=128;    /* to get number of bytes */

	/* Number of Ways */
	NumWays = (CsidReg & 0x3ff) >> 3;
	NumWays += 1;

	/* Get the cacheline size, way size, index size from csidr */
	LineSize = (CsidReg & 0x07) + 4;

	NumSet = CacheSize/NumWays;
	NumSet /= (1 << LineSize);

	Way = 0UL;
	Set = 0UL;

	/* Invalidate all the cachelines */
	for (WayIndex =0; WayIndex < NumWays; WayIndex++) {
		for (SetIndex =0; SetIndex < NumSet; SetIndex++) {
			C7Reg = Way | Set;
			/* Flush by Set/Way */
			__asm__ __volatile__("mcr " \
			ARM_CP15_CLEAN_INVAL_DC_LINE_SW :: "r" (C7Reg));
			Set += (1 << LineSize);
		}
		Way += 0x40000000;
	}

	/* Wait for L1 flush to complete */
	dsb();

	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheEnable() {
	register BT_u32 ctl_reg;
	ctl_reg = rdcp(ARM_CP15_SYS_CONTROL);

	BT_L1DCacheInvalidate();

	ctl_reg |= ARM_CP15_CONTROL_C_BIT;

	wrcp(ARM_CP15_SYS_CONTROL, ctl_reg);

	return BT_ERR_NONE;
}

static BT_ERROR BT_L1DCacheDisable() {
	register BT_u32 CtrlReg;

	/* clean and invalidate the Data cache */
	BT_L1DCacheFlush();


	CtrlReg = rdcp(ARM_CP15_SYS_CONTROL);

	CtrlReg &= ~(ARM_CP15_CONTROL_C_BIT);

	wrcp(ARM_CP15_SYS_CONTROL, CtrlReg);

	return BT_ERR_NONE;
}


BT_ERROR BT_DCacheEnable() {
	BT_L1DCacheEnable();
	BT_L2CacheEnable();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheEnable);

BT_ERROR  BT_DCacheDisable() {
	BT_L2CacheDisable();
	BT_L1DCacheDisable();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheDisable);

BT_ERROR BT_DCacheFlush() {
	BT_L1DCacheFlush();
	BT_L2CacheFlush();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheFlush);

BT_ERROR BT_DCacheFlushLine(void *addr) {
	BT_L1DCacheFlushLine(addr);
	//BT_L2CacheFlushLine(addr);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheFlushLine);

BT_ERROR BT_DCacheInvalidate() {
	BT_L2CacheInvalidate();
	BT_L1DCacheInvalidate();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheInvalidate);

BT_ERROR BT_DCacheInvalidateLine(void *addr) {
	BT_L2CacheInvalidateLine(addr);
	BT_L1DCacheInvalidateLine(addr);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheInvalidateLine);

BT_ERROR BT_DCacheInvalidateRange(void *addr, BT_u32 len) {
	volatile PL310_REGS *pRegs = (PL310_REGS *) g_pregs;
	const BT_u32 cacheline = 32;
	BT_u32 end;

	if(len) {
		end = (BT_u32) addr + len;
		BT_u32 adr = (BT_u32) addr & ~(cacheline - 1);

		wrcp(ARM_CP15_CACHE_SIZE_SEL, 0);

		while(adr < end) {
			pRegs->reg7_inv_pa = adr;
			dsb();
			__asm__ __volatile__("mcr "						\
			ARM_CP15_INVAL_DC_LINE_MVA_POC :: "r" (adr));
			//wrcp(ARM_CP15_INVAL_DC_LINE_MVA_POC, (BT_u32) adr);
			adr += cacheline;
		}
	}

	dsb();

	while(pRegs->reg7_cache_sync);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DCacheInvalidateRange);

static BT_ERROR BT_L1ICacheInvalidate() {
	wrcp(ARM_CP15_CACHE_SIZE_SEL, 1);
	/* invalidate the instruction cache */
	wrcp(ARM_CP15_INVAL_IC_POU, 0);
	/* Wait for L1 invalidate to complete */
	dsb();

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_L1ICacheInvalidate);

BT_ERROR BT_ICacheInvalidate() {
	BT_L2CacheInvalidate();
	BT_L1ICacheInvalidate();
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_ICacheInvalidate);


BT_ERROR arm_pl310_init() {
	if(!g_pregs) {
		g_pregs = (PL310_REGS *) bt_ioremap((void *) (0xF8F02000), BT_SIZE_4K);
	}
	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	.name = "arm,pl310-cache",
	arm_pl310_init,
};
