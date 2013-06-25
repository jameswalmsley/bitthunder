#include <lib/bcd.h>

BT_u8 bcd2bin(BT_u8 ucVal)
{
	return (ucVal & 0x0f) + (ucVal >> 4) * 10;
}

BT_u8 bin2bcd(BT_u8 ucVal)
{
	return ((ucVal / 10) << 4) + ucVal % 10;
}
