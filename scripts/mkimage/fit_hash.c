#include <stdio.h>
#include <stdint.h>
#include <libfdt.h>
#include "crc32.h"
#include "md5.h"
#include "sha1.h"

int calculate_hash(const void *data, int len, const char *algo, unsigned char *value, int *val_len) {
	if(!strcmp(algo, "crc32")) {
		uint32_t crc = crc32(data, len);
		crc = cpu_to_fdt32(crc);
		memcpy(value, &crc, 4);
		*val_len = 4;

	} else if(!strcmp(algo, "sha1")) {
		sha1_context state;
		uint8 digest[20];
		sha1_starts(&state);
		sha1_update(&state, (void *) data, len);
		sha1_finish(&state, digest);

		memcpy(value, digest, 20);
		*val_len = 20;

	} else if(!strcmp(algo, "md5")) {
		md5_state_t	state;
        md5_byte_t	md5_digest[16];

		memset(&state, 0, sizeof(state));

		md5_init(&state);
		md5_append(&state, (md5_byte_t *) data, len);
		md5_finish(&state, md5_digest);

		memcpy(value, md5_digest, 16);
		*val_len = 16;
				
	} else {
		fprintf(stderr, "Unsupported hash algorithm: %s\n", algo);
		return -1;
	}

	return 0;
}

static int fit_set_hash_value(void *fit, int noffset, unsigned char *value, int len) {

	int ret;

	ret = fdt_setprop(fit, noffset, "value", value, len);
	if(ret) {
		-1;
	}

	return 0;
}

static int fit_image_process_hash(void *fit, const char *image_name, int hash_offset, const void *data, int len) {
	
	char hash_value[128];
	int val_len = 0;
	const char *node_name = fdt_get_name(fit, hash_offset, NULL);
	const char *algo = fdt_getprop(fit, hash_offset, "algo", NULL);
	if(!algo) {
		fprintf(stderr, "No algorith specified for %s in %s\n", node_name, image_name);
		return -1;
	}

	if(calculate_hash(data, len, algo, hash_value, &val_len)) {
		fprintf(stderr, "Could not generate a hash (%s) for %s\n", algo, image_name);
		return -1;
	}

	fit_set_hash_value(fit, hash_offset, hash_value, val_len);

	return 0;
}

int fit_image_add_verification_data(void *fit, int image_offset, const char *comment) {

	int err = 0;
	const char *image_name = fdt_get_name(fit, image_offset, &err);
	int noffset, len;
	const void *data;

	data = fdt_getprop(fit, image_offset, "data", &len);

	for(noffset = fdt_first_subnode(fit, image_offset); noffset >= 0; noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret;
		
		node_name = fdt_get_name(fit, noffset, NULL);

		if(!strncmp(node_name, "hash", 4)) {
			fit_image_process_hash(fit, image_name, noffset, data, len);
		}
	}
	

	return 0;
}


int fit_add_verification_data(void *fit, const char *comment) {
	
	int images_offset;
	int noffset;
	int ret;
	
	images_offset = fdt_path_offset(fit, "/images");
	if(images_offset < 0) {
		fprintf(stderr, "Cannot find /images node in image-tree-blob\n");
		return -1;
	}

	for(noffset = fdt_first_subnode(fit, images_offset); noffset >= 0; noffset = fdt_next_subnode(fit, noffset)) {
		ret = fit_image_add_verification_data(fit, noffset, comment);
		if(ret) {
			return ret;
		}
	}

	return 0;
}
