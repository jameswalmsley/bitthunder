/**
 *	bootgen for Zynq platform - An open-source replacement for the Xilinx bootgen tool.
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "bootgen.h"

static char *attributes[] = {
	"init",
	"bootloader",
	"alignment",
	"offset",
	"checksum",
	"encryption",
	"authentication",
};

static void init_header(BOOTROM_HEADER *header) {
	memset(header, 0, sizeof(*header));
	
	header->width_detect = 0xAA995566;
	memcpy(header->identification, "XNLX", 4);
	header->encryption_status = 0;
	header->source_offset = 0x8C0;
	header->reserved_1 = 1;

	int i;
	for(i = 0; i < 256; i++) {
		header->reg_init[i].address = 0xffffffff;
	}
}

static void checksum_header(BOOTROM_HEADER *header) {
	int i = 0;
	uint32_t *word = (uint32_t *) &header->width_detect;

	uint32_t sum = 0;
	   
	for(i = 0; i < 10; i++) {
		sum += word[i];
	}
	
	header->header_checksum = ~sum;
}

int main(int argc, char **argv) {

	struct stat oStat;

	if(stat(argv[1], &oStat)) {
		fprintf(stderr, "Cannot stat file %s\n", argv[1]);
		return -1;
	}

	uint32_t entry_address = strtoul(argv[2], NULL, 16);

	FILE *fp = fopen(argv[1], "rb");
	if(!fp) {
		fprintf(stderr, "Cannot open file %s\n", argv[1]);
		return -1;
	}

	BOOTROM_HEADER *hdr = malloc(sizeof(BOOTROM_HEADER)+oStat.st_size);

	init_header(hdr);
	hdr->image_length = oStat.st_size;
	hdr->total_length = oStat.st_size;
	hdr->user_defined = 0x01010000;
	hdr->entry_address = entry_address;

	fread(&hdr->fsbl, 1, oStat.st_size, fp);

	checksum_header(hdr);

	FILE *out = fopen("BOOT.BIN", "wb");
	fwrite(hdr, 1, sizeof(BOOTROM_HEADER)+oStat.st_size, out);
	fclose(out);

	free(hdr);

	return 0;
}
