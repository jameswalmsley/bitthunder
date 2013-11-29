#include <libfdt.h>
#include <stdint.h>
#include <time.h>

int fit_set_timestamp(void *fit, int noffset, time_t timestamp) {
	uint32_t t;
	t = cpu_to_fdt32(timestamp);
	fdt_setprop(fit, noffset, "timestamp", &t, sizeof(t));
	return 0;
}
