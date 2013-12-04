#ifndef _BT_TIME_H_
#define _BT_TIME_H_

typedef struct _BT_DATETIME {
	BT_u16 	year;		// e.g. 2013
	BT_u8 	month;		// e.g. 1 = Jan 12 = Dec
	BT_u8 	day;		// Day (1-31).
	BT_u8 	hour;		// Hour (0-23).
	BT_u8 	min;		// Minute (0-59).
	BT_u8	second;		// Second (0-59).
} BT_DATETIME;


#endif
