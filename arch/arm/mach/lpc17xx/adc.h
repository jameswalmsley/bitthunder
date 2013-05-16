#ifndef _ADC_H_
#define _ADC_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LPC17xx_ADC_oDeviceInterface;

typedef struct _LPC17xx_ADC_REGS {
	BT_u32	ADCR;					// 0x00		control register

#define	LPC17xx_ADC_ADCR_CHANNELSEL				0x00000001
#define	LPC17xx_ADC_ADCR_CLKDIV_MASK			0x0000FF00
#define	LPC17xx_ADC_ADCR_BURST					0x00010000
#define	LPC17xx_ADC_ADCR_PDN					0x00200000
#define	LPC17xx_ADC_ADCR_START_NOW				0x01000000
#define	LPC17xx_ADC_ADCR_START_PIO2_10			0x02000000
#define	LPC17xx_ADC_ADCR_START_PIO1_27			0x03000000
#define	LPC17xx_ADC_ADCR_START_MAT0_1			0x04000000
#define	LPC17xx_ADC_ADCR_START_MAT0_3			0x05000000
#define	LPC17xx_ADC_ADCR_START_MAT1_0			0x06000000
#define	LPC17xx_ADC_ADCR_START_MAT1_1			0x07000000
#define	LPC17xx_ADC_ADCR_START_RISING_EDGE		0x00000000
#define	LPC17xx_ADC_ADCR_START_FALLING_EDGE		0x08000000

#define	LPC17xx_ADC_MAX_CLOCK					13000000

	BT_u32	ADGDR;					// 0x04		global data register
	BT_STRUCT_RESERVED_u32(0, 0x04, 0x0C);

#define	LPC17xx_ADC_ADGDR_DONE					0x80000000

	BT_u32	ADINTEN;				// 0x0C		interrupt enable register

#define	LPC17xx_ADC_ADINTEN_ADGINTEN			0x00000100

	BT_u32	ADData[8];				// 0x10		conversion result registers

#define	LPC17xx_ADC_ADData_DONE					0x80000000

	BT_u32	ADSTAT;					// 0x30		status register
	BT_u32	ADTRM;					// 0x34		trim register
} LPC17xx_ADC_REGS;

#define ADC0						((LPC17xx_ADC_REGS *) BT_CONFIG_MACH_LPC17xx_ADC0_BASE)

#endif

