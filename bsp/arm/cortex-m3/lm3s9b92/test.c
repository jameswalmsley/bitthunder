#include <bitthunder.h>
#include <string.h>
#include <lib/putc.h>


int main(int argc, char **argv) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_TICK ticks = BT_kTickCount();

	BT_UART_CONFIG oConfig;

	BT_GpioSetDirection(1, BT_GPIO_DIR_OUTPUT);
	BT_GpioSetDirection(0, BT_GPIO_DIR_INPUT);

	BT_HANDLE hUart = BT_DeviceOpen("uart0", &Error);

	oConfig.eMode = BT_UART_MODE_BUFFERED;
	oConfig.ucDataBits = BT_UART_8_DATABITS;
	oConfig.ucParity   = BT_UART_PARITY_NONE;
	oConfig.ucStopBits = BT_UART_ONE_STOP_BIT;
	oConfig.ulBaudrate = 115200;
	oConfig.ulRxBufferSize = 128;
	oConfig.ulTxBufferSize = 128;

	BT_UartSetConfiguration(hUart, &oConfig);

	BT_SetStandardHandle(hUart);

	BT_u8 Data;

	while(1) {
		BT_CharDeviceRead (hUart, 0, 1, &Data);
		BT_CharDeviceWrite(hUart, 0, 1, &Data);
	}

	return 0;
}

