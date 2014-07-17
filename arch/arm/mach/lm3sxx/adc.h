#ifndef _ADC_H_
#define _ADC_H_

#include <bitthunder.h>
#include <bt_struct.h>


extern const BT_IF_DEVICE BT_LM3Sxx_ADC_oDeviceInterface;


typedef struct _LM3Sxx_ADC_REGS {
	BT_u32	ADCACTSS;				// 0x00		active sample sequencer

#define	LM3Sxx_ADC_ADCACTSS_SS0						0x00000001
#define	LM3Sxx_ADC_ADCACTSS_SS1						0x00000002
#define	LM3Sxx_ADC_ADCACTSS_SS2						0x00000004
#define	LM3Sxx_ADC_ADCACTSS_SS3						0x00000008

	BT_u32	ADCRIS;					// 0x04		raw interrupt status

#define	LM3Sxx_ADC_ADCRIS_SS0						0x00000001
#define	LM3Sxx_ADC_ADCRIS_SS1						0x00000002
#define	LM3Sxx_ADC_ADCRIS_SS2						0x00000004
#define	LM3Sxx_ADC_ADCRIS_SS3						0x00000008

	BT_u32	ADCIM;					// 0x08		interrupt mask

#define	LM3Sxx_ADC_ADCIM_SS0						0x00000001
#define	LM3Sxx_ADC_ADCIM_SS1						0x00000002
#define	LM3Sxx_ADC_ADCIM_SS2						0x00000004
#define	LM3Sxx_ADC_ADCIM_SS3						0x00000008

	BT_u32	ADCISC;					// 0x0C		interrupt status and clear
	BT_u32	ADCOSTAT;				// 0x10		interrupt enable register
	BT_u32	ADCEMUX;				// 0x14		interrupt enable register
	BT_u32	ADCUSTAT;				// 0x18		interrupt enable register
	BT_STRUCT_RESERVED_u32(0, 0x18, 0x20);
	BT_u32	ADCSSPRI;				// 0x20		interrupt enable register
	BT_u32	ADCSPC;					// 0x24		interrupt enable register
	BT_u32	ADCPSSI;				// 0x28		interrupt enable register

#define	LM3Sxx_ADC_ADCPSSI_SS0_START				0x00000001
#define	LM3Sxx_ADC_ADCPSSI_SS1_START				0x00000002
#define	LM3Sxx_ADC_ADCPSSI_SS2_START				0x00000004
#define	LM3Sxx_ADC_ADCPSSI_SS3_START				0x00000008

	BT_STRUCT_RESERVED_u32(1, 0x28, 0x30);
	BT_u32	ADCSAC;					// 0x30		interrupt enable register
	BT_u32	ADCDCISC;				// 0x34		interrupt enable register
	BT_u32	ADCCTL;					// 0x38		interrupt enable register
	BT_STRUCT_RESERVED_u32(2, 0x38, 0x40);
	struct {
		BT_u32	SSMUX;				// 0x40; 0x60; 0x80; 0xA0
		BT_u32	SSCTL;				// 0x44; 0x64; 0x84; 0xA4

		#define	LM3Sxx_ADC_MUX_SSCTL_DIFF			0x00000001
		#define	LM3Sxx_ADC_MUX_SSCTL_END			0x00000002
		#define	LM3Sxx_ADC_MUX_SSCTL_INTEN			0x00000004
		#define	LM3Sxx_ADC_MUX_SSCTL_TEMP			0x00000008

		BT_u32	SSFIFO;				// 0x48; 0x68; 0x88; 0xA8
		BT_u32	SSFSTAT;			// 0x4C; 0x6C; 0x8C; 0xAC
		BT_u32	SSOP;				// 0x50; 0x70; 0x90; 0xB0
		BT_u32	SSDC;				// 0x54; 0x74; 0x94; 0xB4
		BT_u32	Dummy3[2];			// 0x58 - 0x5C; 0x78 - 0x7C; 0x98 - 0x9C; 0xB8 - 0xBC
	} Sequencer[4];
	BT_STRUCT_RESERVED_u32(4, 0xBC, 0xD00);

	BT_u32	ADCDCRIC;				// 0xD00

	BT_STRUCT_RESERVED_u32(5, 0xD00, 0xE00);

	BT_u32	ADCDCCTL[8];				// 0xE00

	BT_STRUCT_RESERVED_u32(6, 0xE1C, 0xE40);

	BT_u32	ADCDCCMP[8];				// 0xE40

} LM3Sxx_ADC_REGS;

#define ADC0						((LM3Sxx_ADC_REGS *) BT_CONFIG_MACH_LM3Sxx_ADC0_BASE)
#define ADC1						((LM3Sxx_ADC_REGS *) BT_CONFIG_MACH_LM3Sxx_ADC1_BASE)

#endif

