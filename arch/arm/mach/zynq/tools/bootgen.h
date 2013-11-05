#ifndef _BOOTGEN_H_
#define _BOOTGEN_H_

#include <stdint.h>

typedef struct _REGISTER_INIT {
	uint32_t 	address;
	uint32_t 	value;
} REGISTER_INIT;

typedef struct _BOOTROM_HEADER {
	uint32_t 	interrupts[8];
	uint32_t 	width_detect;
	uint8_t 	identification[4];
	uint32_t 	encryption_status;
	uint32_t 	user_defined;
	uint32_t	source_offset;
	uint32_t	image_length;
	uint32_t 	reserved_0;
	uint32_t	entry_address;
	uint32_t 	total_length;
	uint32_t	reserved_1;
	uint32_t	header_checksum;
	uint32_t	unused_0[21];
	REGISTER_INIT 	reg_init[256];
	uint32_t 	unused_1[8];
	uint8_t		fsbl[1];
} BOOTROM_HEADER;



#endif
