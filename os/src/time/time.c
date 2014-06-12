/**
 *	BitThunder Time Sub-system
 *
 *
 *
 **/


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
