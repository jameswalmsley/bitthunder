#ifndef _BT_CLKDIV_H_
#define _BT_CLKDIV_H_

typedef enum _BT_DIVIDER_TYPE {
	BT_DIVIDER_2_STAGE,
} BT_DIVIDER_TYPE;

typedef struct _BT_DIVIDER_PARAMS {
	BT_DIVIDER_TYPE eType;
	BT_u32 diva_max;			///< Maximum possible divisor for stage a.
	BT_u32 diva_min;
	BT_u32 divb_max;			///< Maximum possible divisor for stage b.
	BT_u32 divb_min;
	BT_u32 diva_val;			///< Recommended value for stage a divider.
	BT_u32 divb_val;			///< Recommended value for stage b divider.
	BT_u32 clk_out;				///< Actual clkout frequency achieved.
} BT_DIVIDER_PARAMS;

BT_ERROR BT_CalculateClockDivider(BT_u32 clkin, BT_u32 clkout, BT_DIVIDER_PARAMS *pDiv);



#endif
