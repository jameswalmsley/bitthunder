#ifndef _TIME_H_
#define _TIME_H_

struct bt_timespec {
	BT_u32	tv_sec;
	BT_u32	tv_nsec;
};

struct bt_timeval {
	BT_u32	tv_sec;				///< Seconds
	BT_u32	tv_usec;			///< Micro-seconds
};

struct bt_timezone {
	BT_u32 	tz_minuteswest;		///< Minutes west of Greenwich.
	BT_u32	tz_dsttime;			///< Type of DST correction.
};

struct bt_tm {
	BT_u32 	tm_sec;
	BT_u32	tm_min;
	BT_u32	tm_hour;
	BT_u32	tm_mday;
	BT_u32	tm_mon;
	BT_u32	tm_year;
	BT_u32	tm_wday;
	BT_u32	tm_yday;
	BT_u32	tm_isdst;
};

static inline BT_BOOL bt_is_leap_year(BT_u32 year) {
	return (!(year % 4) && (year % 100)) || !(year % 400);
}


#endif
