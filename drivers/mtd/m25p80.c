/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Michael Daniel, mdaniel@riegl.com
 *
 * based on the Linux driver by Mike Lavender, mike@steroidmicros.com
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME				("M25P80")
BT_DEF_MODULE_DESCRIPTION		("M25P80 mtd flash driver")
BT_DEF_MODULE_AUTHOR			("Michael Daniel")
BT_DEF_MODULE_EMAIL				("mdaniel@riegl.com")



#define M25P80_WAIT 1

/* Flash opcodes. */
#define	OPCODE_WREN			0x06	/* Write enable */
#define	OPCODE_RDSR			0x05	/* Read status register */
#define	OPCODE_WRSR			0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define OPCODE_QUAD_READ    0x6b        /* Quad read command */
#define	OPCODE_PP			0x02	/* Page program (up to 256 bytes) */
#define OPCODE_QPP          0x32        /* Quad page program */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE			0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID			0x9f	/* Read JEDEC ID */
#define OPCODE_RDFSR		0x70	/* Read Flag Status Register */
#define OPCODE_WREAR		0xc5	/* Write Extended Address Register */

/* Used for SST flashes only. */
#define	OPCODE_BP			0x02	/* Byte program */
#define	OPCODE_WRDI			0x04	/* Write disable */
#define	OPCODE_AAI_WP		0xad	/* Auto address increment word program */

/* Used for Macronix flashes only. */
#define	OPCODE_EN4B			0xb7	/* Enter 4-byte mode */
#define	OPCODE_EX4B			0xe9	/* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define	OPCODE_BRWR			0x17	/* Bank register write */
#define	OPCODE_BRRD			0x16	/* Bank register read */

/* Status Register bits. */
#define	SR_WIP				1	/* Write in progress */
#define	SR_WEL				2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0				4	/* Block protect 0 */
#define	SR_BP1				8	/* Block protect 1 */
#define	SR_BP2				0x10	/* Block protect 2 */
#define	SR_SRWD				0x80	/* SR write protect */

/* Flag Status Register bits. */
#define FSR_RDY				0x80	/* Ready/Busy program erase controller */
#define FSR_ERASE_FAIL		0x20
#define FSR_PROG_FAIL		0x10

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(480 * HZ) 	/* N25Q specs 480s max chip erase */
#define	MAX_CMD_SIZE		5

#define JEDEC_MFR(_jedec_id)	((_jedec_id) >> 16)

#define CFI_MFR_ANY				0xFFFF
#define CFI_ID_ANY				0xFFFF
#define CFI_MFR_CONTINUATION	0x007F

#define CFI_MFR_AMD				0x0001
#define CFI_MFR_AMIC			0x0037
#define CFI_MFR_ATMEL			0x001F
#define CFI_MFR_EON				0x001C
#define CFI_MFR_FUJITSU			0x0004
#define CFI_MFR_HYUNDAI			0x00AD
#define CFI_MFR_INTEL			0x0089
#define CFI_MFR_MACRONIX		0x00C2
#define CFI_MFR_NEC				0x0010
#define CFI_MFR_PMC				0x009D
#define CFI_MFR_SAMSUNG			0x00EC
#define CFI_MFR_SHARP			0x00B0
#define CFI_MFR_SST				0x00BF
#define CFI_MFR_ST				0x0020 /* STMicroelectronics */
#define CFI_MFR_MICRON			0x0020
#define CFI_MFR_TOSHIBA			0x0098
#define CFI_MFR_WINBOND			0x00DA


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	BT_SPI_DEVICE 	*pSpi;
	void*			 lock;
	BT_MTD_INFO		 mtd;
	BT_u16			 page_size;
	BT_u16			 addr_width;
	BT_u8			 erase_opcode;
	BT_u8			*command;
	BT_BOOL			 fast_read;
	BT_u16			 curbank;
	BT_u32			 jedec_id;
	BT_BOOL			 check_fsr;
	BT_BOOL			 shift;
	BT_BOOL		 	 isparallel;
	BT_BOOL			 isstacked;
	BT_u8			 read_opcode;
	BT_u8			 prog_opcode;
	BT_u8			 dummycount;

	BT_ERROR (*_erase) 	(BT_HANDLE flash, BT_MTD_ERASE_INFO *instr);
	BT_ERROR (*_read) 	(BT_HANDLE flash, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf);
	BT_ERROR (*_write) 	(BT_HANDLE flash, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf);
	BT_ERROR (*_lock) 	(BT_HANDLE flash, BT_u64 ofs, BT_u64 len);
	BT_ERROR (*_unlock) (BT_HANDLE flash, BT_u64 ofs, BT_u64 len);
};

/*
 * Internal helper functions
 */

/*
 * Read register, returning its value in the location
 * Returns negative if error occurred.
 */
static inline BT_i32 read_spi_reg(BT_HANDLE flash, BT_u8 code, const char * name)
{
	BT_ERROR retval;
	BT_u8 val;

	retval = BT_SpiWriteThenRead(flash->pSpi, &code, 1, &val, 1);

	if(retval != BT_ERR_NONE) {
		BT_kPrint("M25P80: error %d reading register %s.", retval, name);
		return -retval;
	}
	return val;
}

/*
 * Read flag status register, returning its value in the location
 * Return flag status register value.
 * Returns negative if error occurred.
 */
static BT_i32 read_fsr(BT_HANDLE flash)
{
	return read_spi_reg(flash, OPCODE_RDFSR, "FSR");
}

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static BT_i32 read_sr(BT_HANDLE flash)
{
	return read_spi_reg(flash, OPCODE_RDSR, "SR");
}

/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
static BT_ERROR write_sr(BT_HANDLE flash, BT_u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return BT_SpiWrite(flash->pSpi, flash->command, 2);
}

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static inline BT_ERROR write_enable(BT_HANDLE flash)
{
	BT_u8	code = OPCODE_WREN;
	return BT_SpiWriteThenRead(flash->pSpi, &code, 1, NULL, 0);
}

/*
 * Send write disble instruction to the chip.
 */
static inline BT_ERROR write_disable(BT_HANDLE flash)
{
	BT_u8	code = OPCODE_WRDI;
	return BT_SpiWriteThenRead(flash->pSpi, &code, 1, NULL, 0);
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static BT_ERROR wait_till_ready(BT_HANDLE flash)
{
	int sr, fsr;
	int retry = 48000;

	do {
		sr = read_sr(flash);
		if(sr < 0) {
			break;
		}

		if (!(sr & SR_WIP)) {
			if (flash->check_fsr) {
				fsr = read_fsr(flash);
				if(fsr < 0) {
					break;
				}
				if (!(fsr & FSR_RDY))
					return BT_ERR_GENERIC;
			}
			return BT_ERR_NONE;
		}

		BT_ThreadSleep(5);

	} while (--retry > 0);

	return BT_ERR_BUSY;
}

static BT_ERROR poll_till_ready(BT_HANDLE flash)
{
	int sr;
	do {
		if((sr = read_sr(flash)) < 0)
			return BT_ERR_GENERIC;
	} while(sr & SR_WIP);
	return BT_ERR_NONE;
}

/*
 * Enable/disable 4-byte addressing mode.
 */
static BT_ERROR set_4byte(BT_HANDLE flash, BT_u32 jedec_id, BT_u32 enable)
{
	BT_ERROR ret;
	BT_u8 val;

	switch (JEDEC_MFR(jedec_id)) {
	case CFI_MFR_MACRONIX:
	case 0xEF /* winbond */:
	case CFI_MFR_MICRON:	// TODO: not the best way of getting it enabled for MICRON FLASHES, interferes with STM flashes
		write_enable(flash);
#if M25P80_WAIT
		wait_till_ready(flash);
#else
		poll_till_ready(flash);
#endif
		flash->command[0] = enable ? OPCODE_EN4B : OPCODE_EX4B;
		return BT_SpiWrite(flash->pSpi, flash->command, 1);

	default:
		/* Spansion style */
		flash->command[0] = OPCODE_BRWR;
		flash->command[1] = enable << 7 ;
		ret = BT_SpiWrite(flash->pSpi, flash->command, 2);
		if(BT_ERR_NONE != ret)
		{
			BT_kPrint("M25P80: error in setting addressing info.");
		}

		/* verify the 4 byte mode is enabled */
		flash->command[0] = OPCODE_BRRD;
		if(BT_ERR_NONE != BT_SpiWriteThenRead(flash->pSpi, flash->command, 1, &val, 1))
		{
			BT_kPrint("M25P80: error in getting addressing info.");
		}

		if (val != enable << 7) {
			BT_kPrint("M25P80: fallback to 3-byte address mode, maximum accessible size is 16MB");
			flash->addr_width = 3;
		}
		return ret;
	}

	return BT_ERR_GENERIC;
}

/*
 * Update Extended Address/bank selection Register.
 * Call with flash->lock locked.
 */
static BT_ERROR write_ear(BT_HANDLE flash, BT_u32 addr)
{
	BT_u8 ear;
	int ret;

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash) != BT_ERR_NONE)
		return BT_ERR_BUSY;

	if (flash->mtd.size <= (0x1000000) << flash->shift)
		return 0;

	addr = addr % (BT_u32) flash->mtd.size;
	ear = addr >> 24;

	if ((!flash->isstacked) && (ear == flash->curbank))
		return BT_ERR_NONE;

	if (flash->isstacked && (flash->mtd.size <= 0x2000000))
		return BT_ERR_NONE;

	if (JEDEC_MFR(flash->jedec_id) == 0x01)
		flash->command[0] = OPCODE_BRWR;
	if (JEDEC_MFR(flash->jedec_id) == 0x20) {
		write_enable(flash);
		flash->command[0] = OPCODE_WREAR;
	}
	flash->command[1] = ear;

	ret = BT_SpiWrite(flash->pSpi, flash->command, 2);
	if (ret != BT_ERR_NONE)
		return ret;

	flash->curbank = ear;

	return BT_ERR_NONE;
}

void erase_callback_internal(BT_HANDLE hFlash, BT_MTD_ERASE_INFO * info)
{
	//unsigned long deadline;
	int sr;
	BT_HANDLE flash = hFlash;
	info->state = BT_MTD_ERASE_PENDING;

	do {
		if ((sr = read_sr(flash)) < 0) {
			BT_kPrint("erase_callback: failed to read sr.");
			info->state = BT_MTD_ERASE_FAILED;
			break;
		}
	} while (sr & SR_WIP);

	if(info->state == BT_MTD_ERASE_PENDING)
		info->state = BT_MTD_ERASE_DONE;
}

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static BT_ERROR erase_chip(BT_HANDLE flash)
{
	/* Wait until finished previous write command. */
	if (wait_till_ready(flash) != BT_ERR_NONE)
		return BT_ERR_BUSY;

	if (flash->isstacked)
		flash->pSpi->pMaster->flags &= ~SPI_MASTER_U_PAGE;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;

	BT_SpiWrite(flash->pSpi, flash->command, 1);

	if (flash->isstacked) {
		/* Wait until finished previous write command. */
		if (wait_till_ready(flash) != BT_ERR_NONE)
			return BT_ERR_BUSY;

		flash->pSpi->pMaster->flags |= SPI_MASTER_U_PAGE;

		/* Send write enable, then erase commands. */
		write_enable(flash);

		/* Set up command buffer. */
		flash->command[0] = OPCODE_CHIP_ERASE;

		BT_SpiWrite(flash->pSpi, flash->command, 1);
	}
	return BT_ERR_NONE;
}

static void m25p_addr2cmd(BT_HANDLE flash, BT_u32 addr, BT_u8 *cmd)
{
	int i;

	/* opcode is in cmd[0] */
	for (i = 1; i <= flash->addr_width; i++)
		cmd[i] = addr >> (flash->addr_width * 8 - i * 8);
}

static int m25p_cmdsz(BT_HANDLE flash)
{
	return 1 + flash->addr_width;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static BT_ERROR erase_sector(BT_HANDLE flash, BT_u32 offset)
{
	/* Wait until finished previous write command. */
	if (wait_till_ready(flash) != BT_ERR_NONE)
		return BT_ERR_BUSY;

	/* update Extended Address Register */
	if (write_ear(flash, offset) != BT_ERR_NONE)
		return BT_ERR_GENERIC;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	m25p_addr2cmd(flash, offset, flash->command);

	BT_SpiWrite(flash->pSpi, flash->command, m25p_cmdsz(flash));

	return BT_ERR_NONE;
}

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static BT_ERROR m25p80_erase(BT_HANDLE flash, BT_MTD_ERASE_INFO *instr)
{
	BT_MTD_INFO * mtd = &flash->mtd;
	BT_u32 addr, len, offset;
	BT_u32 rem = instr->len % mtd->erasesize;

	if (rem != 0)
		return BT_ERR_INVALID_VALUE;

	addr = instr->addr;
	len = instr->len;

	BT_kMutexPend(flash->lock, 0);
	/* whole-chip erase? */
	if (len == flash->mtd.size) {
		if (erase_chip(flash) != BT_ERR_NONE) {
			instr->state = BT_MTD_ERASE_FAILED;
			BT_kMutexRelease(flash->lock);
			return BT_ERR_INVALID_RESOURCE;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len) {
			offset = addr;
			if (flash->isparallel == 1)
				offset /= 2;
			if (flash->isstacked == 1) {
				if (offset >= (flash->mtd.size / 2)) {
					offset = offset - (flash->mtd.size / 2);
					flash->pSpi->pMaster->flags |=
							SPI_MASTER_U_PAGE;
				} else
					flash->pSpi->pMaster->flags &=
							~SPI_MASTER_U_PAGE;
			}
			if (erase_sector(flash, offset)) {
				instr->state = BT_MTD_ERASE_FAILED;
				BT_kMutexRelease(flash->lock);
				return BT_ERR_INVALID_RESOURCE;
			}

			addr += mtd->erasesize;
			len -= mtd->erasesize;
		}
	}

	BT_kMutexRelease(flash->lock);

	instr->state = BT_MTD_ERASE_DONE;

	wait_till_ready(flash);
	//poll_till_ready(flash);
	//mtd_erase_callback(flash, instr);
	return BT_ERR_NONE;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static BT_ERROR m25p80_read(BT_HANDLE flash, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf)
{
	BT_SPI_TRANSFER t[2];
	BT_SPI_MESSAGE m;

	BT_SpiMessageInit(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = flash->command;
	//t[0].len = m25p_cmdsz(flash) + (flash->fast_read ? 1 : 0);
	t[0].len = m25p_cmdsz(flash) + flash->dummycount;
	BT_SpiMessageAddTail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	BT_SpiMessageAddTail(&t[1], &m);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash) != BT_ERR_NONE)
	//if (poll_till_ready(flash) != BT_ERR_NONE)
		/* REVISIT status return?? */
		return BT_ERR_BUSY;

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	//opcode = flash->fast_read ? OPCODE_FAST_READ : OPCODE_NORM_READ;
	//flash->command[0] = opcode;
	flash->command[0] = flash->read_opcode;
	m25p_addr2cmd(flash, from, flash->command);

	BT_SpiSync(flash->pSpi, &m);

	//*retlen = m.actual_length - m25p_cmdsz(flash) - (flash->fast_read ? 1 : 0);
	*retlen = m.actual_length - m25p_cmdsz(flash) - flash->dummycount;

	return BT_ERR_NONE;
}


static BT_ERROR m25p80_read_ext(BT_HANDLE flash, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf)
{
	BT_u32 addr = from;
	BT_u32 offset = from;
	BT_u32 read_len = 0;
	BT_u32 actual_len = 0;
	BT_u32 read_count = 0;
	BT_u32 rem_bank_len = 0;
	BT_u8 bank = 0;

#define OFFSET_16_MB 0x1000000

	BT_kMutexPend(flash->lock, 0);

	while (len) {
		bank = addr / (OFFSET_16_MB << flash->shift);
		rem_bank_len = ((OFFSET_16_MB << flash->shift) * (bank + 1)) -
				addr;
		offset = addr;
		if (flash->isparallel == 1)
			offset /= 2;
		if (flash->isstacked == 1) {
			if (offset >= (flash->mtd.size / 2)) {
				offset = offset - (flash->mtd.size / 2);
				flash->pSpi->pMaster->flags |= SPI_MASTER_U_PAGE;
			} else {
				flash->pSpi->pMaster->flags &= ~SPI_MASTER_U_PAGE;
			}
		}
		write_ear(flash, offset);
		if (len < rem_bank_len)
			read_len = len;
		else
			read_len = rem_bank_len;

		m25p80_read(flash, offset, read_len, &actual_len, buf);

		addr += actual_len;
		len -= actual_len;
		buf += actual_len;
		read_count += actual_len;
	}

	*retlen = read_count;

	BT_kMutexRelease(flash->lock);
	return BT_ERR_NONE;
}

static BT_ERROR mtd_read_wrapper(BT_HANDLE flash, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf)
{
	if(!flash->_read)
		return BT_ERR_INVALID_HANDLE;

	return flash->_read(flash, from, len, retlen, buf);
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static BT_ERROR m25p80_write(BT_HANDLE flash, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf)
{
	BT_u32 page_offset, page_size;
	BT_SPI_TRANSFER t[2];
	BT_SPI_MESSAGE m;

	BT_SpiMessageInit(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash);
	BT_SpiMessageAddTail(&t[0], &m);

	t[1].tx_buf = buf;
	BT_SpiMessageAddTail(&t[1], &m);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash) != BT_ERR_NONE)
		return BT_ERR_BUSY;

	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	//flash->command[0] = OPCODE_PP;
	flash->command[0] = flash->prog_opcode;
	m25p_addr2cmd(flash, to, flash->command);

	page_offset = to & (flash->page_size - 1);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= flash->page_size) {
		t[1].len = len;

		BT_SpiSync(flash->pSpi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash);
	} else {
		BT_u32 i;

		/* the size of data remaining on the first page */
		page_size = flash->page_size - page_offset;

		t[1].len = page_size;
		BT_SpiSync(flash->pSpi, &m);

		*retlen = m.actual_length - m25p_cmdsz(flash);

		/* write everything in flash->page_size chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > flash->page_size)
				page_size = flash->page_size;

			/* write the next page to flash */
			/*
			if (flash->isparallel)
				m25p_addr2cmd(flash, to + i/2, flash->command);
			else
				m25p_addr2cmd(flash, to + i, flash->command);
			*/
			m25p_addr2cmd(flash, ((to + i) >> flash->shift), flash->command);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;


			if (poll_till_ready(flash) != BT_ERR_NONE)
				return BT_ERR_BUSY;

			write_enable(flash);

			BT_SpiSync(flash->pSpi, &m);

			*retlen += m.actual_length - m25p_cmdsz(flash);
		}
	}

	return BT_ERR_NONE;
}

static BT_ERROR m25p80_write_ext(BT_HANDLE flash, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf)
{
	BT_u32 addr = to;
	BT_u32 offset = to;
	BT_u32 write_len = 0;
	BT_u32 actual_len = 0;
	BT_u32 write_count = 0;
	BT_u32 rem_bank_len = 0;
	BT_u8 bank = 0;

#define OFFSET_16_MB 0x1000000

	BT_kMutexPend(flash->lock, 0);
	while (len) {
		bank = addr / (OFFSET_16_MB << flash->shift);
		rem_bank_len = ((OFFSET_16_MB << flash->shift) * (bank + 1)) -
				addr;
		offset = addr;

		//if (flash->isparallel == 1) xilinx
		//	offset /= 2;
		if (flash->isstacked == 1) {
			if (offset >= (flash->mtd.size / 2)) {
				offset = offset - (flash->mtd.size / 2);
				flash->pSpi->pMaster->flags |= SPI_MASTER_U_PAGE;
			} else {
				flash->pSpi->pMaster->flags &= ~SPI_MASTER_U_PAGE;
			}
		}
		//write_ear(flash, offset);
		write_ear(flash, (offset >> flash->shift));
		if (len < rem_bank_len)
			write_len = len;
		else
			write_len = rem_bank_len;

		m25p80_write(flash, offset, write_len, &actual_len, buf);

		addr += actual_len;
		len -= actual_len;
		buf += actual_len;
		write_count += actual_len;
	}

	*retlen = write_count;

	BT_kMutexRelease(flash->lock);
	return BT_ERR_NONE;
}

// TODO: !!!!!!!!!!!!!!!!
static BT_ERROR sst_write(BT_HANDLE flash, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf)
{
	BT_SPI_TRANSFER t[2];
	BT_SPI_MESSAGE m;
	BT_u32 actual;
	int cmd_sz;
	BT_ERROR ret = BT_ERR_NONE;

	BT_SpiMessageInit(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = m25p_cmdsz(flash);
	BT_SpiMessageAddTail(&t[0], &m);

	t[1].tx_buf = buf;
	BT_SpiMessageAddTail(&t[1], &m);

	BT_kMutexPend(flash->lock, 0);

	/* Wait until finished previous write command. */
	ret = wait_till_ready(flash);
	if (ret != BT_ERR_NONE);
		goto time_out;

	write_enable(flash);

	actual = to % 2;
	/* Start write from odd address. */
	if (actual) {
		flash->command[0] = OPCODE_BP;
		m25p_addr2cmd(flash, to, flash->command);

		/* write one byte. */
		t[1].len = 1;
		BT_SpiSync(flash->pSpi, &m);
		ret = wait_till_ready(flash);
		if (ret != BT_ERR_NONE)
			goto time_out;
		*retlen += m.actual_length - m25p_cmdsz(flash);
	}
	to += actual;

	flash->command[0] = OPCODE_AAI_WP;
	m25p_addr2cmd(flash, to, flash->command);

	/* Write out most of the data here. */
	cmd_sz = m25p_cmdsz(flash);
	for (; actual < len - 1; actual += 2) {
		t[0].len = cmd_sz;
		/* write two bytes. */
		t[1].len = 2;
		t[1].tx_buf = buf + actual;

		BT_SpiSync(flash->pSpi, &m);
		ret = wait_till_ready(flash);
		if (ret != BT_ERR_NONE)
			goto time_out;
		*retlen += m.actual_length - cmd_sz;
		cmd_sz = 1;
		to += 2;
	}
	write_disable(flash);
	ret = wait_till_ready(flash);
	if (ret != BT_ERR_NONE)
		goto time_out;

	/* Write out trailing byte if it exists. */
	if (actual != len) {
		write_enable(flash);
		flash->command[0] = OPCODE_BP;
		m25p_addr2cmd(flash, to, flash->command);
		t[0].len = m25p_cmdsz(flash);
		t[1].len = 1;
		t[1].tx_buf = buf + actual;

		BT_SpiSync(flash->pSpi, &m);
		ret = wait_till_ready(flash);
		if (ret != BT_ERR_NONE)
			goto time_out;
		*retlen += m.actual_length - m25p_cmdsz(flash);
		write_disable(flash);
	}

time_out:
	BT_kMutexRelease(flash->lock);
	return ret;
}

static BT_ERROR mtd_write_wrapper(BT_HANDLE flash, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf)
{
	if(!flash->_write)
		return BT_ERR_INVALID_HANDLE;

	return flash->_write(flash, to, len, retlen, buf);
}


static BT_ERROR m25p80_lock(BT_HANDLE flash, BT_u64 ofs, BT_u64 len)
{
	//BT_MTD_INFO *mtd = &flash->mtd;
	BT_u32 offset = ofs;
	BT_u8 status_old, status_new;
	BT_ERROR res = BT_ERR_NONE;

	BT_kMutexPend(flash->lock, 0);
	/* Wait until finished previous command */
	if (wait_till_ready(flash) != BT_ERR_NONE) {
		res = BT_ERR_BUSY;
		goto err;
	}

	status_old = read_sr(flash);

	if (offset < flash->mtd.size-(flash->mtd.size/2))
		status_new = status_old | SR_BP2 | SR_BP1 | SR_BP0;
	else if (offset < flash->mtd.size-(flash->mtd.size/4))
		status_new = (status_old & ~SR_BP0) | SR_BP2 | SR_BP1;
	else if (offset < flash->mtd.size-(flash->mtd.size/8))
		status_new = (status_old & ~SR_BP1) | SR_BP2 | SR_BP0;
	else if (offset < flash->mtd.size-(flash->mtd.size/16))
		status_new = (status_old & ~(SR_BP0|SR_BP1)) | SR_BP2;
	else if (offset < flash->mtd.size-(flash->mtd.size/32))
		status_new = (status_old & ~SR_BP2) | SR_BP1 | SR_BP0;
	else if (offset < flash->mtd.size-(flash->mtd.size/64))
		status_new = (status_old & ~(SR_BP2|SR_BP0)) | SR_BP1;
	else
		status_new = (status_old & ~(SR_BP2|SR_BP1)) | SR_BP0;

	/* Only modify protection if it will not unlock other areas */
	if ((status_new&(SR_BP2|SR_BP1|SR_BP0)) >
					(status_old&(SR_BP2|SR_BP1|SR_BP0))) {
		write_enable(flash);
		if (write_sr(flash, status_new) < 0) {
			res = BT_ERR_GENERIC;
			goto err;
		}
	}

err:
	BT_kMutexRelease(flash->lock);
	return res;
}

static BT_ERROR m25p80_unlock(BT_HANDLE flash, BT_u64 ofs, BT_u64 len)
{
	//BT_MTD_INFO *mtd = &flash->mtd;
	BT_u32 offset = ofs;
	BT_u8 status_old, status_new;
	BT_ERROR res = BT_ERR_NONE;

	BT_kMutexPend(flash->lock, 0);

	/* Wait until finished previous command */
	if (wait_till_ready(flash) != BT_ERR_NONE) {
		res = BT_ERR_BUSY;
		goto err;
	}

	status_old = read_sr(flash);

	if (offset+len > flash->mtd.size-(flash->mtd.size/64))
		status_new = status_old & ~(SR_BP2|SR_BP1|SR_BP0);
	else if (offset+len > flash->mtd.size-(flash->mtd.size/32))
		status_new = (status_old & ~(SR_BP2|SR_BP1)) | SR_BP0;
	else if (offset+len > flash->mtd.size-(flash->mtd.size/16))
		status_new = (status_old & ~(SR_BP2|SR_BP0)) | SR_BP1;
	else if (offset+len > flash->mtd.size-(flash->mtd.size/8))
		status_new = (status_old & ~SR_BP2) | SR_BP1 | SR_BP0;
	else if (offset+len > flash->mtd.size-(flash->mtd.size/4))
		status_new = (status_old & ~(SR_BP0|SR_BP1)) | SR_BP2;
	else if (offset+len > flash->mtd.size-(flash->mtd.size/2))
		status_new = (status_old & ~SR_BP1) | SR_BP2 | SR_BP0;
	else
		status_new = (status_old & ~SR_BP0) | SR_BP2 | SR_BP1;

	/* Only modify protection if it will not lock other areas */
	if ((status_new&(SR_BP2|SR_BP1|SR_BP0)) <
					(status_old&(SR_BP2|SR_BP1|SR_BP0))) {
		write_enable(flash);
		if (write_sr(flash, status_new) < 0) {
			res = BT_ERR_GENERIC;
			goto err;
		}
	}

err:
	BT_kMutexRelease(flash->lock);
	return res;
}

/****************************************************************************/

/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	BT_u32		jedec_id;
	BT_u16      ext_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	BT_u32		sector_size;
	BT_u16		n_sectors;

	BT_u16		page_size;
	BT_u16		addr_width;

	BT_u16		flags;
//#define	SECT_4K			0x01		/* OPCODE_BE_4K works uniformly */
//#define	M25P_NO_ERASE	0x02		/* No erase command needed */
//#define	SECT_32K		0x04		/* OPCODE_BE_32K */
//#define E_FSR			0x08		/* Flag SR exists for flash */
#define     SECT_4K         0x01                /* OPCODE_BE_4K works uniformly */
#define     M25P_NO_ERASE   0x02                /* No erase command needed */
#define     SST_WRITE       0x04                /* use SST byte programming */
#define     SECT_32K		0x10                /* OPCODE_BE_32K */
#define		E_FSR           0x08                /* Flag SR exists for flash */
};

#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)	\
	((BT_u32)&(struct flash_info) {				\
		.jedec_id = (_jedec_id),				\
		.ext_id = (_ext_id),					\
		.sector_size = (_sector_size),				\
		.n_sectors = (_n_sectors),				\
		.page_size = 256,					\
		.flags = (_flags),					\
	})

#define CAT25_INFO(_sector_size, _n_sectors, _page_size, _addr_width)	\
	((BT_u32)&(struct flash_info) {				\
		.sector_size = (_sector_size),			\
		.n_sectors = (_n_sectors),				\
		.page_size = (_page_size),				\
		.addr_width = (_addr_width),				\
		.flags = M25P_NO_ERASE,					\
	})

/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static const struct bt_spi_device_id m25p_ids[] = {
        /* Atmel -- some are (confusingly) marketed as "DataFlash" */
        { "at25fs010",  INFO(0x1f6601, 0, 32 * 1024,   4, SECT_4K) },
        { "at25fs040",  INFO(0x1f6604, 0, 64 * 1024,   8, SECT_4K) },

        { "at25df041a", INFO(0x1f4401, 0, 64 * 1024,   8, SECT_4K) },
        { "at25df321a", INFO(0x1f4701, 0, 64 * 1024,  64, SECT_4K) },
        { "at25df641",  INFO(0x1f4800, 0, 64 * 1024, 128, SECT_4K) },

        { "at26f004",   INFO(0x1f0400, 0, 64 * 1024,  8, SECT_4K) },
        { "at26df081a", INFO(0x1f4501, 0, 64 * 1024, 16, SECT_4K) },
        { "at26df161a", INFO(0x1f4601, 0, 64 * 1024, 32, SECT_4K) },
        { "at26df321",  INFO(0x1f4700, 0, 64 * 1024, 64, SECT_4K) },

        { "at45db081d", INFO(0x1f2500, 0, 64 * 1024, 16, SECT_4K) },

        /* EON -- en25xxx */
        { "en25f32", INFO(0x1c3116, 0, 64 * 1024,  64, SECT_4K) },
        { "en25p32", INFO(0x1c2016, 0, 64 * 1024,  64, 0) },
        { "en25q32b", INFO(0x1c3016, 0, 64 * 1024,  64, 0) },
        { "en25p64", INFO(0x1c2017, 0, 64 * 1024, 128, 0) },
        { "en25q64", INFO(0x1c3017, 0, 64 * 1024, 128, SECT_4K) },
        { "en25qh256", INFO(0x1c7019, 0, 64 * 1024, 512, 0) },

        /* Everspin */
        { "mr25h256", CAT25_INFO(  32 * 1024, 1, 256, 2) },

        /* GigaDevice */
        { "gd25q32", INFO(0xc84016, 0, 64 * 1024,  64, SECT_4K) },
        { "gd25q64", INFO(0xc84017, 0, 64 * 1024, 128, SECT_4K) },

        /* Intel/Numonyx -- xxxs33b */
        { "160s33b",  INFO(0x898911, 0, 64 * 1024,  32, 0) },
        { "320s33b",  INFO(0x898912, 0, 64 * 1024,  64, 0) },
        { "640s33b",  INFO(0x898913, 0, 64 * 1024, 128, 0) },

        /* Macronix */
        { "mx25l2005a",  INFO(0xc22012, 0, 64 * 1024,   4, SECT_4K) },
        { "mx25l4005a",  INFO(0xc22013, 0, 64 * 1024,   8, SECT_4K) },
        { "mx25l8005",   INFO(0xc22014, 0, 64 * 1024,  16, 0) },
        { "mx25l1606e",  INFO(0xc22015, 0, 64 * 1024,  32, SECT_4K) },
        { "mx25l3205d",  INFO(0xc22016, 0, 64 * 1024,  64, 0) },
        { "mx25l6405d",  INFO(0xc22017, 0, 64 * 1024, 128, 0) },
        { "mx25l12805d", INFO(0xc22018, 0, 64 * 1024, 256, 0) },
        { "mx25l12855e", INFO(0xc22618, 0, 64 * 1024, 256, 0) },
        { "mx25l25635e", INFO(0xc22019, 0, 64 * 1024, 512, 0) },
        { "mx25l25655e", INFO(0xc22619, 0, 64 * 1024, 512, 0) },
        { "mx66l51235l", INFO(0xc2201a, 0, 64 * 1024, 1024, 0) },

        /* Micron */
        { "n25q064",  INFO(0x20ba17, 0, 64 * 1024, 128, 0) },
        { "n25q128a11",  INFO(0x20bb18, 0, 64 * 1024, 256, 0) },
        { "n25q128a13",  INFO(0x20ba18, 0, 64 * 1024, 256, 0) },
        { "n25q256a", INFO(0x20ba19, 0, 64 * 1024, 512, SECT_4K) },
        /* Numonyx flash n25q128 - FIXME check the name */
        { "n25q128",   INFO(0x20bb18, 0, 64 * 1024, 256, 0) },
        { "n25q128a11",  INFO(0x20bb18, 0, 64 * 1024, 256, E_FSR) },
        { "n25q128a13",  INFO(0x20ba18, 0, 64 * 1024, 256, E_FSR) },
        { "n25q256a13", INFO(0x20ba19,  0, 64 * 1024,  512, SECT_4K | E_FSR) },
        { "n25q256a11", INFO(0x20bb19,  0, 64 * 1024,  512, SECT_4K | E_FSR) },
        { "n25q512a13", INFO(0x20ba20,  0, 64 * 1024,  1024, SECT_4K | E_FSR) },
        { "n25q512a11", INFO(0x20bb20,  0, 64 * 1024,  1024, SECT_4K | E_FSR) },
        { "n25q00aa13", INFO(0x20ba21,  0, 64 * 1024,  2048, SECT_4K | E_FSR) },

        /* Spansion -- single (large) sector size only, at least
         * for the chips listed here (without boot sectors).
         */
        { "s25sl032p",  INFO(0x010215, 0x4d00,  64 * 1024,  64, 0) },
        { "s25sl064p",  INFO(0x010216, 0x4d00,  64 * 1024, 128, 0) },
        { "s25fl256s0", INFO(0x010219, 0x4d00, 256 * 1024, 128, 0) },
        { "s25fl256s1", INFO(0x010219, 0x4d01,  64 * 1024, 512, 0) },
        { "s25fl512s",  INFO(0x010220, 0x4d00, 256 * 1024, 256, 0) },
        { "s70fl01gs",  INFO(0x010221, 0x4d00, 256 * 1024, 256, 0) },
        { "s25sl12800", INFO(0x012018, 0x0300, 256 * 1024,  64, 0) },
        { "s25sl12801", INFO(0x012018, 0x0301,  64 * 1024, 256, 0) },
        { "s25fl129p0", INFO(0x012018, 0x4d00, 256 * 1024,  64, 0) },
        { "s25fl129p1", INFO(0x012018, 0x4d01,  64 * 1024, 256, 0) },
        { "s25sl004a",  INFO(0x010212,      0,  64 * 1024,   8, 0) },
        { "s25sl008a",  INFO(0x010213,      0,  64 * 1024,  16, 0) },
        { "s25sl016a",  INFO(0x010214,      0,  64 * 1024,  32, 0) },
        { "s25sl032a",  INFO(0x010215,      0,  64 * 1024,  64, 0) },
        { "s25sl064a",  INFO(0x010216,      0,  64 * 1024, 128, 0) },
        { "s25fl016k",  INFO(0xef4015,      0,  64 * 1024,  32, SECT_4K) },
        /* s25fl064k supports 4KiB, 32KiB and 64KiB sectors erase size. */
        /* To support JFFS2, the minimum erase size is 8KiB(>4KiB). */
        /* And thus, the sector size of s25fl064k is set to 32KiB for */
        /* JFFS2 support. */
        { "s25fl064k",  INFO(0xef4017,      0,  64 * 1024, 128, SECT_32K) },

        /* SST -- large erase sizes are "overlays", "sectors" are 4K */
        { "sst25vf040b", INFO(0xbf258d, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
        { "sst25vf080b", INFO(0xbf258e, 0, 64 * 1024, 16, SECT_4K | SST_WRITE) },
        { "sst25vf016b", INFO(0xbf2541, 0, 64 * 1024, 32, SECT_4K | SST_WRITE) },
        { "sst25vf032b", INFO(0xbf254a, 0, 64 * 1024, 64, SECT_4K | SST_WRITE) },
        { "sst25vf064c", INFO(0xbf254b, 0, 64 * 1024, 128, SECT_4K) },
        { "sst25wf512",  INFO(0xbf2501, 0, 64 * 1024,  1, SECT_4K | SST_WRITE) },
        { "sst25wf010",  INFO(0xbf2502, 0, 64 * 1024,  2, SECT_4K | SST_WRITE) },
        { "sst25wf020",  INFO(0xbf2503, 0, 64 * 1024,  4, SECT_4K | SST_WRITE) },
        { "sst25wf040",  INFO(0xbf2504, 0, 64 * 1024,  8, SECT_4K | SST_WRITE) },
        { "sst25wf080",  INFO(0xbf2505, 0, 64 * 1024,  16, SECT_4K | SST_WRITE) },

        /* ST Microelectronics -- newer production may have feature updates */
        { "m25p05",  INFO(0x202010,  0,  32 * 1024,   2, 0) },
        { "m25p10",  INFO(0x202011,  0,  32 * 1024,   4, 0) },
        { "m25p20",  INFO(0x202012,  0,  64 * 1024,   4, 0) },
        { "m25p40",  INFO(0x202013,  0,  64 * 1024,   8, 0) },
        { "m25p80",  INFO(0x202014,  0,  64 * 1024,  16, 0) },
        { "m25p16",  INFO(0x202015,  0,  64 * 1024,  32, 0) },
        { "m25p32",  INFO(0x202016,  0,  64 * 1024,  64, 0) },
        { "m25p64",  INFO(0x202017,  0,  64 * 1024, 128, 0) },
        { "m25p128", INFO(0x202018,  0, 256 * 1024,  64, 0) },
        { "n25q032", INFO(0x20ba16,  0,  64 * 1024,  64, 0) },

        { "m25p05-nonjedec",  INFO(0, 0,  32 * 1024,   2, 0) },
        { "m25p10-nonjedec",  INFO(0, 0,  32 * 1024,   4, 0) },
        { "m25p20-nonjedec",  INFO(0, 0,  64 * 1024,   4, 0) },
        { "m25p40-nonjedec",  INFO(0, 0,  64 * 1024,   8, 0) },
        { "m25p80-nonjedec",  INFO(0, 0,  64 * 1024,  16, 0) },
        { "m25p16-nonjedec",  INFO(0, 0,  64 * 1024,  32, 0) },
        { "m25p32-nonjedec",  INFO(0, 0,  64 * 1024,  64, 0) },
        { "m25p64-nonjedec",  INFO(0, 0,  64 * 1024, 128, 0) },
        { "m25p128-nonjedec", INFO(0, 0, 256 * 1024,  64, 0) },

        { "m45pe10", INFO(0x204011,  0, 64 * 1024,    2, 0) },
        { "m45pe80", INFO(0x204014,  0, 64 * 1024,   16, 0) },
        { "m45pe16", INFO(0x204015,  0, 64 * 1024,   32, 0) },

        { "m25pe20", INFO(0x208012,  0, 64 * 1024,  4,       0) },
        { "m25pe80", INFO(0x208014,  0, 64 * 1024, 16,       0) },
        { "m25pe16", INFO(0x208015,  0, 64 * 1024, 32, SECT_4K) },

        { "m25px32",    INFO(0x207116,  0, 64 * 1024, 64, SECT_4K) },
        { "m25px32-s0", INFO(0x207316,  0, 64 * 1024, 64, SECT_4K) },
        { "m25px32-s1", INFO(0x206316,  0, 64 * 1024, 64, SECT_4K) },
        { "m25px64",    INFO(0x207117,  0, 64 * 1024, 128, 0) },

        /* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
        { "w25x10", INFO(0xef3011, 0, 64 * 1024,  2,  SECT_4K) },
        { "w25x20", INFO(0xef3012, 0, 64 * 1024,  4,  SECT_4K) },
        { "w25x40", INFO(0xef3013, 0, 64 * 1024,  8,  SECT_4K) },
        { "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K) },
        { "w25x16", INFO(0xef3015, 0, 64 * 1024,  32, SECT_4K) },
        { "w25x32", INFO(0xef3016, 0, 64 * 1024,  64, SECT_4K) },
        { "w25q32", INFO(0xef4016, 0, 64 * 1024,  64, SECT_4K) },
        { "w25q32dw", INFO(0xef6016, 0, 64 * 1024,  64, SECT_4K) },
        { "w25x64", INFO(0xef3017, 0, 64 * 1024, 128, SECT_4K) },
        /* Winbond -- w25q "blocks" are 64K, "sectors" are 32KiB */
        /* w25q64 supports 4KiB, 32KiB and 64KiB sectors erase size. */
        /* To support JFFS2, the minimum erase size is 8KiB(>4KiB). */
        /* And thus, the sector size of w25q64 is set to 32KiB for */
        /* JFFS2 support. */
        { "w25q64", INFO(0xef4017, 0, 64 * 1024, 128, SECT_32K) },
        { "w25q80", INFO(0xef5014, 0, 64 * 1024,  16, SECT_4K) },
        { "w25q80bl", INFO(0xef4014, 0, 64 * 1024,  16, SECT_4K) },
        { "w25q128", INFO(0xef4018, 0, 64 * 1024, 256, SECT_4K) },
        { "w25q256", INFO(0xef4019, 0, 64 * 1024, 512, SECT_4K) },

        /* Catalyst / On Semiconductor -- non-JEDEC */
        { "cat25c11", CAT25_INFO(  16, 8, 16, 1) },
        { "cat25c03", CAT25_INFO(  32, 8, 16, 2) },
        { "cat25c09", CAT25_INFO( 128, 8, 32, 2) },
        { "cat25c17", CAT25_INFO( 256, 8, 32, 2) },
        { "cat25128", CAT25_INFO(2048, 8, 64, 2) },
        { },
};


static const struct bt_spi_device_id *jedec_probe(BT_SPI_DEVICE *spi)
{
	BT_i32 		tmp;
	BT_u8 		code = OPCODE_RDID;
	BT_u8		id[5];
	BT_u32		jedec;
	BT_u16		ext_jedec;
	struct flash_info *info;

	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */

	if(BT_SpiWriteThenRead(spi,&code,1,id,5) != BT_ERR_NONE)
		return NULL;

	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	ext_jedec = id[3] << 8 | id[4];

	//BT_kPrint("jedec: 0x%x, ext_jedec: 0x%x", jedec, ext_jedec);

	for (tmp = 0; tmp < BT_ARRAY_SIZE(m25p_ids) - 1; tmp++) {
		info = (void *)m25p_ids[tmp].driver_data;
		if (info->jedec_id == jedec) {
			if (info->ext_id != 0 && info->ext_id != ext_jedec)
				continue;
			return &m25p_ids[tmp];
		}
	}
	return NULL;
}

static BT_ERROR mtd_cleanup(BT_HANDLE hMTD) {
	BT_kFree(hMTD->pSpi);
	BT_DestroyHandle(hMTD);
	return BT_ERR_NONE;
}

static const BT_DEV_IF_MTD oMTDInterface = {
	.pfnErase		= m25p80_erase,
	.pfnRead		= mtd_read_wrapper,
	.pfnWrite		= mtd_write_wrapper,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pPowerIF = NULL,
	.eConfigType = BT_DEV_IF_T_MTD,
	.unConfigIfs = {
		.pMtdIF = &oMTDInterface,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.eType = BT_HANDLE_T_DEVICE,
	.pfnCleanup = mtd_cleanup,
};

static BT_HANDLE mtd_probe(BT_SPI_DEVICE *spi, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	int i=0;
	BT_HANDLE flash = NULL;
	BT_ERROR Error = BT_ERR_NONE;

	spi->bits_per_word = 32;
	spi->chip_select = 0;
	spi->mode = spi->pMaster->mode_bits;
	spi->max_speed_hz = 50000000;

	Error = BT_SpiSetup(spi);
	if(Error != BT_ERR_NONE) {
		BT_kPrint("mtd_probe: SpiSetup error %d", Error);
		goto err_out_free_spi;
	}

	// get jedec id
	const struct bt_spi_device_id *jid = jedec_probe(spi);
	struct flash_info * info;
	const struct bt_spi_device_id *id;
	if(jid == NULL) {
		goto err_out_free_spi;
	}
	else {
		/*
		 * JEDEC knows better, so overwrite platform ID. We
		 * can't trust partitions any longer, but we'll let
		 * mtd apply them anyway, since some partitions may be
		 * marked read-only, and we don't want to lose that
		 * information, even if it's not 100% accurate.
		 */
		BT_kPrint( "M25P80: found %s", jid->name);
		id = jid;
		info = (void *)jid->driver_data;
	}

	flash = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!flash) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out_free_spi;
	}

	flash->fast_read = BT_FALSE;
	// TODO: get property fast read

	flash->command = BT_kMalloc(MAX_CMD_SIZE + flash->fast_read ? 1 : 0);
	if(!flash->command) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out_free_flash;
	}
	flash->pSpi = spi;
	flash->lock = BT_kMutexCreate();

	/*
	 * Atmel, SST and Intel/Numonyx serial flash tend to power
	 * up with the software protection bits set
	 */

	if (JEDEC_MFR(info->jedec_id) == CFI_MFR_ATMEL ||
		JEDEC_MFR(info->jedec_id) == CFI_MFR_INTEL ||
		JEDEC_MFR(info->jedec_id) == CFI_MFR_SST) {
		write_enable(flash);
		write_sr(flash, 0);
	}

	// Bitthunder specific
	flash->mtd.hMtd = flash;

	// End Bitthunder specific

	flash->mtd.type = BT_MTD_NORFLASH;
	flash->mtd.writesize = 1;
	flash->mtd.flags = BT_MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;

	/* single chip */
	flash->shift = 0;
	flash->isstacked = 0;
	flash->isparallel = 0;

	// set erase and read function
	flash->_erase = m25p80_erase;
	flash->_read = m25p80_read_ext;

	/* flash protection support for STmicro chips */
	if (JEDEC_MFR(info->jedec_id) == CFI_MFR_ST) {
		flash->_lock = m25p80_lock;
		flash->_unlock = m25p80_unlock;
	}

	/* sst flash chips use AAI word program */
	//if (JEDEC_MFR(info->jedec_id) == CFI_MFR_SST)
	if(info->flags & SST_WRITE)
		flash->_write = sst_write;
	else
		flash->_write = m25p80_write_ext;


	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096 << flash->shift;
	} else if (info->flags & SECT_32K) {
		flash->erase_opcode = OPCODE_BE_32K;
		flash->mtd.erasesize = 32768 << flash->shift;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

	// TODO: check for preferred erase size attribute!!
	flash->erase_opcode = OPCODE_SE;
	flash->mtd.erasesize = info->sector_size;

	flash->read_opcode = OPCODE_NORM_READ;
	flash->prog_opcode = OPCODE_PP;
	flash->dummycount = 0;

	if (info->flags & M25P_NO_ERASE)
		flash->mtd.flags |= BT_MTD_NO_ERASE;

	if (info->flags & E_FSR)
		flash->check_fsr = 1;

	flash->jedec_id = info->jedec_id;
	//ppdata.of_node = spi->dev.of_node;
	//flash->mtd.dev.parent = &spi->dev;
	flash->page_size = info->page_size;
	flash->mtd.writebufsize = flash->page_size;

	flash->fast_read = BT_TRUE;

	if(flash->fast_read) {
		flash->read_opcode = OPCODE_FAST_READ;
		flash->dummycount = 1;
	}

	/*
	if(spi->pMaster->flags & SPI_MASTER_QUAD_MODE) {
		flash->read_opcode = OPCODE_QUAD_READ;
		flash->prog_opcode = OPCODE_QPP;
		flash->dummycount = 1;
	}*/


	if (info->addr_width)
		flash->addr_width = info->addr_width;
	else {
		/* enable 4-byte addressing if the device exceeds 16MiB */

		if (flash->mtd.size > 0x1000000) {
			flash->addr_width = 4;
			if(set_4byte(flash, info->jedec_id, 1) != BT_ERR_NONE) {
				BT_kPrint("M25P80: falling back to 3-byte address mode.");
				flash->addr_width = 3;
			}
		} else
			flash->addr_width = 3;
	}

	flash->mtd.name = id->name;

	BT_kPrint("M25P80: %s (%lld Kbytes)", id->name, (long long)flash->mtd.size >> 10);

	BT_kPrint("M25P80: mtd .name = %s, .size = 0x%llx (%lldMiB) .erasesize = 0x%.8x (%uKiB) .numeraseregions = %d",
			flash->mtd.name,
			(long long)flash->mtd.size, (long long)(flash->mtd.size >> 20),
			flash->mtd.erasesize, flash->mtd.erasesize / 1024,
			flash->mtd.numeraseregions);


	if(flash->mtd.numeraseregions)
		for(i=0; i<flash->mtd.numeraseregions; i++)
			BT_kPrint("mtd.eraseregions[%d] = { .offset = 0x%llx, .erasesize = 0x%.8x (%uKiB), .numblocks = %d }",
					i, (long long)flash->mtd.eraseregions[i].offset,
					flash->mtd.eraseregions[i].erasesize,
					flash->mtd.eraseregions[i].erasesize / 1024,
					flash->mtd.eraseregions[i].numblocks);



	if(pError)
		*pError = Error;

	BT_MTD_RegisterDevice(flash, "mtd0", &flash->mtd);


	BT_kPrint("M25P80: probe succeeded");

	return flash;


err_out_free_flash:
	BT_DestroyHandle(flash);
err_out_free_spi:
	BT_kFree(spi);
	if(pError)
		*pError = Error;
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 	= "n25q128",
	.eType 	= BT_DRIVER_SPI,
	.pfnSPIProbe = mtd_probe,
};
