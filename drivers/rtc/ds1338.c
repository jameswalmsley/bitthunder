/**
 *	DS1338 and derivatives driver.
 *
 *	@author Robert Steinbauer <rsteinbauer@riegl.co.at>
 **/

#include <bitthunder.h>
#include <lib/bcd.h>
#include <rtc/bt_rtc.h>

BT_DEF_MODULE_NAME				("Dallas 1338 RTC")
BT_DEF_MODULE_DESCRIPTION		("Maxim I2C RTC")
BT_DEF_MODULE_AUTHOR			("Robert Steinbauer")
BT_DEF_MODULE_EMAIL				("rsteinbauer@riegl.co.at")

#define DS1338_REG_SECS         0x00    /* 00-59 */
#define DS1338_REG_MIN          0x01    /* 00-59 */
#define DS1338_REG_HOUR         0x02    /* 00-23, or 1-12{am,pm} */
#define DS1338_REG_WDAY         0x03    /* 01-07 */
#define DS1338_REG_MDAY         0x04    /* 01-31 */
#define DS1338_REG_MONTH        0x05    /* 01-12 */
#define DS1338_REG_YEAR         0x06    /* 00-99 */

#define DS1338_BIT_12HR          0x40    /* in REG_HOUR */
#define DS1338_BIT_PM            0x20    /* in REG_HOUR */
#define DS1338_BIT_CENTURY_EN    0x80    /* in REG_HOUR */
#define DS1338_BIT_CENTURY       0x80    /* in REG_MONTH */


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	BT_RTC_INFO		 oInfo;
	BT_I2C_CLIENT	 oClient;
	BT_I2C_MESSAGE	 oMessages[2];
};

static BT_ERROR rtc_cleanup(BT_HANDLE hRtc) {
	return BT_ERR_NONE;
}

static BT_ERROR get_time(BT_HANDLE hRtc, struct rtctime *t) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_u8 ucBuffer[8];
	BT_u8 buf = 0;

	hRtc->oMessages[0].addr 	= hRtc->oClient.addr;
	hRtc->oMessages[0].len  	= 1;
	hRtc->oMessages[0].buf  	= &buf;
	hRtc->oMessages[0].flags    = 0;

	hRtc->oMessages[1].addr 	= hRtc->oClient.addr;
	hRtc->oMessages[1].len		= 7;
	hRtc->oMessages[1].buf		= ucBuffer;
	hRtc->oMessages[1].flags 	= BT_I2C_M_RD;

	/* read the RTC date and time registers all at once */
	BT_I2C_Transfer(hRtc->oClient.pBus, hRtc->oMessages, 2, &Error);

	t->tm_sec = bcd2bin(ucBuffer[DS1338_REG_SECS] & 0x7f);
	t->tm_min = bcd2bin(ucBuffer[DS1338_REG_MIN] & 0x7f);
	BT_u8 uctmp = ucBuffer[DS1338_REG_HOUR] & 0x3f;
	t->tm_hour = bcd2bin(uctmp);
	t->tm_wday = bcd2bin(ucBuffer[DS1338_REG_WDAY] & 0x07);
	t->tm_mday = bcd2bin(ucBuffer[DS1338_REG_MDAY] & 0x3f);
	uctmp = ucBuffer[DS1338_REG_MONTH] & 0x1f;
	t->tm_mon = bcd2bin(uctmp);

	/* assume 20YY not 19YY, and ignore DS1337_BIT_CENTURY */
	t->tm_year = bcd2bin(ucBuffer[DS1338_REG_YEAR]) + 2000;

	/* initial clock setting can be undefined */
	return Error;
 }

static BT_ERROR set_time(BT_HANDLE hRtc, struct rtctime *t) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_u8 ucBuffer[8];

	ucBuffer[0] = 0x00;
	ucBuffer[1+DS1338_REG_SECS] = bin2bcd(t->tm_sec) & 0x7F;
	ucBuffer[1+DS1338_REG_MIN] = bin2bcd(t->tm_min);
	ucBuffer[1+DS1338_REG_HOUR] = bin2bcd(t->tm_hour);
	ucBuffer[1+DS1338_REG_WDAY] = bin2bcd(t->tm_wday);
	ucBuffer[1+DS1338_REG_MDAY] = bin2bcd(t->tm_mday);
	ucBuffer[1+DS1338_REG_MONTH] = bin2bcd(t->tm_mon);

	BT_u8 uctmp = t->tm_year - 2000;
	ucBuffer[1+DS1338_REG_YEAR] = bin2bcd(uctmp);

	hRtc->oMessages[0].addr 	= hRtc->oClient.addr;
	hRtc->oMessages[0].len		= 8;
	hRtc->oMessages[0].buf		= ucBuffer;
	hRtc->oMessages[0].flags 	= 0;

	BT_I2C_Transfer(hRtc->oClient.pBus, hRtc->oMessages, 1, &Error);

	return Error;
}

static const BT_DEV_IF_RTC rtc_ops = {
	.pfnGetTime			= get_time,
	.pfnSetTime			= set_time,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_RTC,
	.unConfigIfs = {
		.pRTCIF = &rtc_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = rtc_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static BT_HANDLE rtc_probe(BT_HANDLE hBus, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	struct rtctime Time;

	BT_I2C_BUS *pBus = BT_I2C_GetBusObject(hBus);

	BT_HANDLE hRtc = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hRtc) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hRtc->oClient.pBus = pBus;

	const BT_RESOURCE *pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_ADDR, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	hRtc->oClient.addr = pResource->ulStart;

	BT_kPrint("Probing RTC DS1338 on I2C-bus %d and I2C-address 0x%02x", hRtc->oClient.pBus->ulBusID, hRtc->oClient.addr);

	get_time(hRtc, &Time);

	BT_kPrint("Time : %02d:%02d:%02d", Time.tm_hour, Time.tm_min, Time.tm_sec);
	BT_kPrint("Date : %02d.%02d.%04d", Time.tm_mday, Time.tm_mon, Time.tm_year);

	hRtc->oInfo.pDevice = pDevice;

	BT_RTCRegisterDevice(hRtc, &hRtc->oInfo);

	return hRtc;

err_free_out:
	BT_DestroyHandle(hRtc);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 	= "dallas,ds1338",
	.eType 	= BT_DRIVER_I2C,
	.pfnI2CProbe = rtc_probe,
};
