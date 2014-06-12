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






#endif
