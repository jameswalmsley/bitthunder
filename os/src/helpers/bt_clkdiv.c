#include <bitthunder.h>

BT_ERROR BT_CalculateClockDivider(BT_u32 clkin, BT_u32 clkout, BT_DIVIDER_PARAMS *pDiv) {

	BT_u32 calc_clkout, best_error = 0xFFFFFFFF, calc_error;

	BT_u32 a=0,b=0;

	for(a=pDiv->diva_min; a < pDiv->diva_max; a++) {
		b = clkin / (clkout * (a+1));			// Calculate stage B divider value.
		if(b > pDiv->divb_max || b < pDiv->divb_min) {
			continue;
		}

		BT_u32 rem = clkin % (clkout * (a+1));
		if(rem > ((clkout * (a+1)) / 2)) {
			b += 1;
		}

		calc_clkout = clkin / (b * (a+1));		// Calculate the actual output freq.

		if(clkout > calc_clkout) {
			calc_error = clkout - calc_clkout;
		} else {
			calc_error = calc_clkout - clkout;
		}

		if(best_error > calc_error) {
			pDiv->diva_val = a+1;
			pDiv->divb_val = b;
			pDiv->clk_out = calc_clkout;
			best_error = calc_error;
		}
	}

	return BT_ERR_NONE;
}
