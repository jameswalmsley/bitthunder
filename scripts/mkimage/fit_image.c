#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <libfdt.h>
#include <fcntl.h>
#include <time.h>

int mmap_fdt(const char *filename, void **blobp, struct stat *sbuf) {
	int fd = open(filename, O_RDWR);
	if(fd < 0){
		fprintf(stderr, "Cannot open %s\n", filename);
		return -1;
	}

	if(fstat(fd, sbuf) < 0) {
		fprintf(stderr, "Cannot stat %s\n", filename);
		return -1;
	}

	void *ptr = mmap(0, sbuf->st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s\n", filename);
		return -1;
	}

	if(fdt_check_header(ptr)) {
		fprintf(stderr, "Invalid FIT blob : %s\n", filename);
		return -1;
	}

	*blobp = ptr;

	return fd;
}

int fit_image(int argc, char **argv) {
	if(argc - optind != 2) {
		fprintf(stderr, 
				"Usage:\n"
				"      %s -f [fit_source].its [fit_image].itb\n", argv[0]);
		return -1;
	}

	char *source = argv[optind];
	char *dest   = argv[optind+1];

	char commandline[4096];
	snprintf(commandline, 4096, "../dtc/dtc -S dts -O dtb -p 500 %s -o %s", source, dest);

	if(system(commandline) < 0) {
		fprintf(stderr, "Failed to execute the dtc: %s (%s)\n", commandline, strerror(errno));
		return -1;
	}

	void *ptr;
	struct stat sbuf;

	int tfd = mmap_fdt(dest, &ptr, &sbuf);
	if(tfd < 0) {
		goto err_mmap;
	}

	if(fit_add_verification_data(ptr, "no comment")) {
		fprintf(stderr, "Failed to add hashes to FIT blob\n");
		goto err_add_hashes;
	}

	fit_set_timestamp(ptr, 0, sbuf.st_mtime);

	return 0;


err_add_hashes:
	munmap(ptr, sbuf.st_size);
err_mmap:
	
	return -1;
}


int fit_list_configs(void *fit, int configs_offset) {
	int node;
	int count;

	const char *default_cfg = fdt_getprop(fit, configs_offset, "default", NULL);
	printf("\n\n  Default configuration: %s\n", default_cfg);

	for(count = 0,node = fdt_first_subnode(fit, configs_offset); node >= 0; node = fdt_next_subnode(fit, node), count++) {
		printf("\n === Configuration (%d) - %s ===\n", count, fdt_get_name(fit, node, NULL));
		const char *description = fdt_getprop(fit, node, "description", NULL);
		printf("  Description   : %s\n", description);
		const char *kernel = fdt_getprop(fit, node, "kernel", NULL);
		printf("  Kernel        : %s\n", kernel);

		const char *fpga = fdt_getprop(fit, node, "fpga", NULL);
		if(fpga) {

			const char *fpga_device = fdt_getprop(fit, node, "fpga_device", NULL);
			printf("  fpga          : %s\n", fpga);
			printf("  fpga_device   : %s\n", fpga_device);
		}
	}

}

int fit_list_images(void *fit, int images_offset) {
	int node;
	int count;

	const char *description;

	for(count = 0,node = fdt_first_subnode(fit, images_offset); node >= 0; node = fdt_next_subnode(fit, node), count++) {
		
		printf("\n  === Image (%d) - %s ===\n", count, fdt_get_name(fit, node, NULL));

		description = fdt_getprop(fit, node, "description", NULL);
		if(!description) {
			description = "Unavailable";
		}

		const char *type = fdt_getprop(fit, node, "type", NULL);
		const char *compression = fdt_getprop(fit, node, "compression", NULL);
		int data_len = 0;
		const void *data = fdt_getprop(fit, node, "data", &data_len);
		char *length = "Unavailable";
		char len_buf[32];
		if(data) {
			snprintf(len_buf, 32, "%d", data_len);
			length = len_buf;
		}

		const char *arch = fdt_getprop(fit, node, "arch", NULL);
		const char *os = fdt_getprop(fit, node, "os", NULL);
		
		const fdt32_t *load_addr = fdt_getprop(fit, node, "load", NULL);
		int32_t load = 0;
		if(load_addr) {
			load = fdt32_to_cpu(*load_addr);
		}

		const fdt32_t *entry_addr = fdt_getprop(fit, node, "entry", NULL);
		int32_t entry = 0;
		if(entry_addr) {
			entry = fdt32_to_cpu(*entry_addr);
		}
   
		printf("  Description   : %s\n", description);
		printf("  Type          : %s\n", type);
		printf("  Compression   : %s\n", compression);
		printf("  Length (bytes): %s\n", length);
		if(arch) {
		printf("  Architecture  : %s\n", arch);
		}
		if(os) {
			printf("  OS            : %s\n", os);
		}
		if(load_addr) {
			printf("  Load Address  : 0x%08x\n", load);
		}

		if(entry_addr) {
			printf("  Entry Address : 0x%08x\n", entry);
		}

		// Check all hashes.
		int hash_node;
		int hash_count = 0;
		for(hash_count = 0, hash_node = fdt_first_subnode(fit, node); 
			hash_node >= 0; hash_node = fdt_next_subnode(fit, hash_node), hash_count++) {
			
			printf("  === Hash %d ===\n", hash_count);
	
			const char *algo = fdt_getprop(fit, hash_node, "algo", NULL);
			int val_len;
			const unsigned char *value = fdt_getprop(fit, hash_node, "value", &val_len);

			char digest[20*2+2];
			char val[8];
			int i;

			strcpy(digest, "");

			for(i = 0; i < val_len; i++) {
				sprintf(val, "%02x", value[i]);
				strcat(digest, val);
			}

			digest[2*val_len] = '\0';

			printf("    Algorithm   : %s\n", algo);
			printf("    Digest      : %s\n", digest);

			calculate_hash(data, data_len, algo, digest, &i);

			char *verify = "FAILED";

			if(!memcmp(value, digest, val_len)) {
				verify = "OK";
			}

			printf("    Verify      : %s\n", verify);
		}
	}
}

int fit_list_image(int argc, char **argv) {
	if(argc - optind != 1) {
		fprintf(stderr,
				"Usage:\n"
				"      %s -l [fit_image].itb\n", argv[0]);
		return -1;
	}

	char *filename = argv[optind];

	void *fit;
	struct stat sbuf;

	if(mmap_fdt(filename, &fit, &sbuf) < 0) {
		printf("failed to mmap file\n");
		return -1;
	}

	time_t t;
	int noffset = fdt_path_offset(fit, "/");

	const char *description = fdt_getprop(fit, noffset, "description", NULL);
	if(!description) {
		description = "Unavailable";
	}

	const char *timestamp = "Unavailable";

	const fdt32_t *time_fdt = fdt_getprop(fit, noffset, "timestamp", NULL);
	if(time_fdt) {
		t = fdt32_to_cpu(*time_fdt);
		timestamp = ctime(&t);
	}

	printf("FIT description : %s\n", description);
	printf("Generated       : %s", timestamp);

	int images_offset = fdt_path_offset(fit, "/images");
	if(images_offset < 0) {
		fprintf(stderr, "Can't find /images node in %s\n", filename);
		return -1;
	}
	

	fit_list_images(fit, images_offset);

	int configs_offset = fdt_path_offset(fit, "/configurations");

	fit_list_configs(fit, configs_offset);
	
	return 0;
}
