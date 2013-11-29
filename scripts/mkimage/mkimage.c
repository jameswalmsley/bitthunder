/**
 *	BitThunder / BootThunder mkimage tool.
 *
 *	This tool is used to generate uBoot compliant FIT images using the DTC.
 *
 **/

#include <stdio.h>
#include <getopt.h>
#include <errno.h>

void usage(int argc, char **argv) {
	fprintf(stderr, 
			"Usage: %s -l [image_file]\n"
			"       -l => list image header information.\n"
			"\n", argv[0]);
	
	fprintf(stderr,
			"       %s -f [fit_source].its [fit_image].itb\n"
			"       -f => Generate a flattened image tree from an its file.\n", argv[0]);
		
}

int main(int argc, char **argv) {

	int c;
	int bFitImage = 0;
	int bListImage = 0;
	while((c = getopt(argc, argv, "lf")) >= 0) {
		switch(c) {
		case 'l':
			bListImage = 1;
			break;

		case 'f':
			bFitImage = 1;
			break;

		case '?':
		default:
			fprintf(stderr, "Invalid commandline argument -%c.", optopt);
			usage(argc, argv);
			return -1;
		}
	}

	if(bListImage) {
		return fit_list_image(argc, argv);
	}

	if(bFitImage) {
		return fit_image(argc, argv);
	}

	usage(argc, argv);

	return 0;
}
