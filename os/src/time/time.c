/**
 *	BitThunder Time Sub-system
 *
 *
 *
 **/
#include <bitthunder.h>

struct _bt_systime {
	BT_HANDLE 			hRTC;						///< RTC device being used to synchronise.
	struct bt_timeval	sys_time;					///< Real-time at sync point.
	BT_TICK				sys_ticks;					///< Number of kernel ticks at sync point-
	BT_u32				sys_ticks_us;				///< uSeconds since previous tick.
};

static struct _bt_systime g_systime = { NULL };

static const BT_u8 days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static const BT_u16 ydays[2][13] = {
	/* Normal years */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

/*
 * The number of days in the month.
 */
BT_u32 bt_month_days(unsigned int month, unsigned int year) {
	return days_in_month[month] + (bt_is_leap_year(year) && month == 1);
}
BT_EXPORT_SYMBOL(bt_month_days);

BT_u32 bt_year_days(unsigned int day, unsigned int month, unsigned int year) {
	return ydays[bt_is_leap_year(year)][month] + day-1;
}
BT_EXPORT_SYMBOL(bt_year_days);


BT_u32 bt_mktime(BT_u32 year0, BT_u32 mon0, BT_u32 day, BT_u32 hour, BT_u32 min, BT_u32 sec) {

	BT_u32 mon = mon0, year = year0;

	if((BT_s32) (mon -=2)) {
		mon += 12;
		year -= 1;
	}

	return ((((BT_u32)
			  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
			  year*365 - 719499
				 ) * 24 + hour
				) * 60 + min
		) * 60 + sec;
}

void bt_time_to_tm(unsigned long time, struct bt_rtc_time *tm) {
	unsigned int month, year;
	int days;

	days = time / 86400;
	time -= (unsigned int) days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + bt_is_leap_year(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - bt_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month;
	tm->tm_mday = days + 1;

	tm->tm_hour = time / 3600;
	time -= tm->tm_hour * 3600;
	tm->tm_min = time / 60;
	tm->tm_sec = time - tm->tm_min * 60;

	tm->tm_isdst = 0;
}
BT_EXPORT_SYMBOL(bt_time_to_tm);

BT_ERROR bt_gettimeofday(struct bt_timeval *tv, struct bt_timezone *tz) {

	BT_TICK oTicks = BT_GetKernelTick() - g_systime.sys_ticks;	///< Kernel ticks since sync.
	BT_u32 seconds = 0;
	BT_u32 usecs = 0;

	BT_kEnterCritical();
	{
		usecs = BT_GetSystemTimerOffset();
		oTicks = BT_GetKernelTick();
	}
	BT_kExitCritical();

	while(usecs >= 1000) {
		oTicks += 1;
		usecs -= 1000;
	}

	seconds = oTicks / 1000;
	usecs = ((oTicks % 1000) * 1000) + usecs;

	if(tv) {
		tv->tv_sec 	= g_systime.sys_time.tv_sec + seconds;
		tv->tv_usec = g_systime.sys_time.tv_usec + usecs;
		if(tv->tv_usec > 1000000) {
			tv->tv_sec += 1;
			tv->tv_usec -= 1000000;
		}
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_gettimeofday);

BT_ERROR bt_settimeofday(const struct bt_timeval *tv, const struct bt_timezone *tz) {

	BT_u32 usecs;
	BT_u32 oTicks;

	BT_kEnterCritical();
	{
		usecs = BT_GetSystemTimerOffset();
		oTicks = BT_GetKernelTick();
	}
	BT_kExitCritical();

	while(usecs >= 1000) {
		oTicks += 1;
		usecs -= 1000;
	}

	if(tv) {
		g_systime.sys_time.tv_sec 	= tv->tv_sec;
		g_systime.sys_time.tv_usec 	= tv->tv_usec;
		g_systime.sys_ticks 		= oTicks;
		g_systime.sys_ticks_us 		= usecs;
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_settimeofday);
