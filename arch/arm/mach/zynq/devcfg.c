/**
 *	Zynq Device Configuration driver.
 *
 *	Exposes a file/IO (char device) interface to the devcfg hardware.
 *
 **/

#include <bitthunder.h>
#include <string.h>
#include "devcfg.h"
#include "slcr.h"

BT_DEF_MODULE_NAME				("Zynq Device Configuration")
BT_DEF_MODULE_DESCRIPTION		("Provides filesystem access to the devcfg hardware")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	volatile ZYNQ_DEVCFG_REGS	*pRegs;
	volatile ZYNQ_SLCR_REGS		*pSLCR;
	BT_BOOL	bEndianSwap;
	BT_u32 residue_len;
	BT_u32 offset;
	BT_u8 residue_buf[3];
};

static BT_BOOL g_bInUse = BT_FALSE;

static BT_ERROR devcfg_cleanup(BT_HANDLE h) {
	g_bInUse = BT_FALSE;

	zynq_slcr_unlock(h->pSLCR);
	zynq_slcr_postload_fpga(h->pSLCR);
	zynq_slcr_lock(h->pSLCR);

	bt_iounmap(h->pRegs);
	bt_iounmap(h->pSLCR);

	return BT_ERR_NONE;
}


static void devcfg_reset_pl(BT_HANDLE hDevcfg) {

	hDevcfg->pRegs->CTRL |= CTRL_PCFG_PROG_B;				// Setting PCFG_PROGB signal to high

	while(!(hDevcfg->pRegs->STATUS & STATUS_PCFG_INIT)) {	// Wait for PL for reset
		;
	}

	hDevcfg->pRegs->CTRL &= ~CTRL_PCFG_PROG_B;				// Setting PCFG_PROGB signal to low

	while((hDevcfg->pRegs->STATUS & STATUS_PCFG_INIT)) {	// Wait for PL for status set
		;
	}

	hDevcfg->pRegs->CTRL |= CTRL_PCFG_PROG_B;
	while(!(hDevcfg->pRegs->STATUS & STATUS_PCFG_INIT)) {	// Wait for PL for status set
		;
	}
}


/**
 *	This assumes a single write request will be generated.
 **/
static BT_u32 devcfg_write(BT_HANDLE hDevcfg, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer, BT_ERROR *pError) {

	BT_u32 user_count = ulSize;

	BT_u32 kmem_size = ulSize + hDevcfg->residue_len;
	bt_paddr_t kmem = bt_page_alloc_coherent(kmem_size);
	if(!kmem) {
		bt_printf("Cannot allocate memory");
		*pError = BT_ERR_NO_MEMORY;
		return 0;
	}

	BT_u8 *buf = (BT_u8 *) bt_phys_to_virt(kmem);

	// Collect stragglers from last time (0 to 3 bytes).
	memcpy(buf, hDevcfg->residue_buf, hDevcfg->residue_len);

	// Copy the user data.
	memcpy(buf + hDevcfg->residue_len, pBuffer, ulSize);

	// Include straggles in total to be counted.
	ulSize += hDevcfg->residue_len;

	// Check if header?
	if(hDevcfg->offset == 0 && ulSize > 4) {
		BT_u32 i;
		for(i = 0; i < ulSize - 4; i++) {
			if(!memcmp(buf + i, "\x66\x55\x99\xAA", 4)) {
				BT_kPrint("xdevcfg: found normal sync word.");
				hDevcfg->bEndianSwap = 0;
				break;
			}

			if(!memcmp(buf + i, "\xAA\x99\x55\x66", 4)) {
				BT_kPrint("xdevcfg: found byte-swapped sync word.");
				hDevcfg->bEndianSwap = 1;
				break;
			}
		}

		if(i != ulSize - 4) {
			ulSize -= i;
			memmove(buf, buf + i, ulSize);	// ulSize - i ??
		}
	}

	// Save stragglers for next time.
	hDevcfg->residue_len = ulSize % 4;
	ulSize -= hDevcfg->residue_len;
	memcpy(hDevcfg->residue_buf, buf + ulSize, hDevcfg->residue_len);

	// Fixup the endianness
	if(hDevcfg->bEndianSwap) {
		BT_u32 i;
		for (i = 0; i < ulSize; i += 4) {
			BT_u32 *p = (BT_u32 *) &buf[i];
			p[0] = __builtin_bswap32(p[0]);
		}
	}

	// Transfer the data.

	hDevcfg->pRegs->DMA_SRC_ADDR = (BT_u32 ) kmem | 1;
	hDevcfg->pRegs->DMA_DST_ADDR = 0xFFFFFFFF;

	BT_u32 transfer_len = 0;
	if(ulSize % 4) {
		transfer_len = (ulSize / 4) + 1;
	} else {
		transfer_len = (ulSize / 4);
	}

	hDevcfg->pRegs->DMA_SRC_LEN = transfer_len;
	hDevcfg->pRegs->DMA_DST_LEN = 0;

	while(!(hDevcfg->pRegs->INT_STS & INT_STS_DMA_DONE_INT)) {
		BT_ThreadYield();
	}

	hDevcfg->pRegs->INT_STS = INT_STS_DMA_DONE_INT;	// Clear DMA_DONE status

	hDevcfg->offset += user_count;

	bt_page_free_coherent(kmem, kmem_size);

	return user_count;
}


static BT_u32 devcfg_read(BT_HANDLE hDevcfg, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	return 0;
}

static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= devcfg_read,
	.pfnWrite 	= devcfg_write,
};

static BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = devcfg_cleanup,
	.eType = BT_HANDLE_T_FILE,
	.pFileIF = &oFileOperations,
};

static BT_HANDLE devcfg_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	if(g_bInUse) {
		Error = BT_ERR_GENERIC;
		goto err_set_out;
	}

	g_bInUse = BT_TRUE;

	BT_HANDLE hDevcfg = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hDevcfg) {
		goto err_set_out;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hDevcfg->pRegs = (volatile ZYNQ_DEVCFG_REGS *) bt_ioremap(pResource->ulStart, sizeof(ZYNQ_DEVCFG_REGS));

	hDevcfg->pRegs->UNLOCK = 0x757BDF0D;				// Unlock the DEVCFG interface.

	hDevcfg->pRegs->INT_STS = 0xFFFFFFFF;				// Clear all interrupt status signals.

	hDevcfg->pRegs->CTRL |= CTRL_PCFG_PROG_B;
	hDevcfg->pRegs->CTRL |= CTRL_PCAP_MODE;				// Enable PCAP transfer mode.
	hDevcfg->pRegs->CTRL |= CTRL_PCAP_PR;				// Select PCAP for reconfiguration, (disables ICAP).
	hDevcfg->pRegs->CTRL &= ~CTRL_QUARTER_PCAP_RATE;	// Set full bandwidth PCAP loading rate.

	hDevcfg->pRegs->MCTRL &= ~MCTRL_PCAP_LPBK; 			// Ensure internal PCAP looback is disabled.

	hDevcfg->pSLCR = (volatile ZYNQ_SLCR_REGS *) bt_ioremap(ZYNQ_SLCR_BASE, sizeof(ZYNQ_SLCR_REGS));

	zynq_slcr_unlock(hDevcfg->pSLCR);
	zynq_slcr_preload_fpga(hDevcfg->pSLCR);
	zynq_slcr_lock(hDevcfg->pSLCR);

	devcfg_reset_pl(hDevcfg);

	if(pError) {
		*pError = BT_ERR_NONE;
	}

	return hDevcfg;

err_free_out:
	BT_DestroyHandle(hDevcfg);

err_set_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF devcfg_driver = {
	.name 		= "zynq,devcfg",
	.pfnProbe	= devcfg_probe,
};
