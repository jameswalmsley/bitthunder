/**
 *	Defines the Reset and Clock Control Register definitions for LM3Sxx
 *
 *
 *
 **/
#ifndef _IOCON_H_
#define _IOCON_H_

#include <bitthunder.h>
#include "gpio.h"


#define	LM3Sxx_PORTA			0
#define	LM3Sxx_PORTB			1
#define	LM3Sxx_PORTC			2
#define	LM3Sxx_PORTD			3
#define	LM3Sxx_PORTE			4
#define	LM3Sxx_PORTF			5
#define	LM3Sxx_PORTG			6
#define	LM3Sxx_PORTH			7
#define	LM3Sxx_PORTJ			8

void BT_LM3Sxx_SetIOConfig(BT_u32 ulPort, BT_u32 ulPin, BT_u32 ulFunction, BT_u32 ulMode, BT_u32 ulAnalogInput, BT_u32 ulOpenDrain);

#endif

