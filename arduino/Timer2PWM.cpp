/*
Timer2PWM.h - Library for PWM converter.
J. Castillejo, May 2016.
Released into the public domain.
*/

#include "Arduino.h"
#include "Timer2PWM.h"


byte Timer2PWMClass::begin(byte Mode, long Frequency, bool Invert, byte Setpoint)
//timer2 is used to provide pwm output
//mode1: 8bit,fast PWM, bridge/half bridge, counter from 0 to OCR2A, any frequency, PWM output in OC2B, toggle in OC2A. warning: frequency is for ONE transistor/inductor. real frequency is 2x
//mode2: 7bit, phase correct PWM,push-pull, counter from 0 to 255, freqs:  PWM output in OC2A and OC2B(shifted)
{
	byte Prescaler; //CS22  CS21  CS20
	long N; //prescaler factor: 1, 8, 32, 64, 128, 256, 1024
					//define timer2 pwm pins as outputs. pins vary depending on board
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		pinMode(10, OUTPUT); //OC2A
		pinMode(9, OUTPUT); //OC2B
	#else
		pinMode(3, OUTPUT); //OC2B
		pinMode(11, OUTPUT); //OC2A
	#endif
	_Mode = Mode;
	//Timer2 setup
	//
	//TCCR2A
	//COM2A1 COM2A0 COM2B1 COM2B0 ----- ----- WGM21 WGM20
	//
	//TCCR2B
	//FOC2A   FOC2B  -----  ----- WGM22  CS22  CS21  CS20
	//  
	//  CS22   CS21   CS20
	//   0		0	   0   No clock source (Timer/Counter stopped
	//   0      0      1   N=1:		CLK (16MHz)/1 (no prescaling)
	//   0      1      0   N=8:		CLK (16MHz)/8
	//   0      1      1   N=32:	CLK (16MHz)/32
	//   1      0      0   N=64:	CLK (16MHz)/64
	//   1      0      1   N=128:	CLK (16MHz)/128
	//   1      1      0   N=256:	CLK (16MHz)/256
	//   1      1      1   N=1024:	CLK (16MHz)/1024
	//
	//FOC2A, FOC2B unused in PWM mode
	switch (_Mode)
	{
	case 1:
		
		//	        ------
		//	|	   |      |
		//	| 	   |      |
		//	 ------        ------ OC2A f=Frequency
		//
		//   --     --     --
		//  |  |   |  |   |  |
		//  |  |   |  |   |  |
		//      ---    ---    --- OC2B f=2xFrequency
		//
		//WGM22   WGM21  WGM20
		//   1      1      1  Fast PWM, Top=OCR2A, Update of UCR2B at Bottom, TOV Flag set on Top
		//
		//COM2A1 COM2A0
		//   0      1    WGM22 = 1: Toggle OC2A on Compare Match
		//
		//COM2B1 COM2B0
		//   1      0    Clear OC2B on Compare Match (non-inverting mode)  prototype 1, 1.1
		//   1      1    Set OC2B on Compare Match (inverting mode)        prototype 1.2
		//PWM Freq=16000000/(2*N*(OCR2A+1))
		//optimal frequencies (0<=OCR2A<=255): 31250, 3907, 977, 489, 245, 123, 31 Hz

		{
			if (!Invert) {
				TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); //non-inverting pwm output
			}
			else {
				TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(COM2B0) | _BV(WGM21) | _BV(WGM20); //inverting pwm output
			}
			if (Invert)	TCCR2A = TCCR2A | _BV(COM2B0);
			if (Frequency >= 31250) { //31250-4000000 Hz
				N = 1;
				Prescaler = _BV(CS20);
			}
			else if (Frequency >= 3907) { //3907-500000 Hz
				N = 8;
				Prescaler = _BV(CS21);
			}
			else if (Frequency >= 977) { //977-125000 Hz
				N = 32;
				Prescaler = _BV(CS21) | _BV(CS20);
			}
			else if (Frequency >= 489) { //489-62500 Hz
				N = 64;
				Prescaler = _BV(CS22);
			}
			else if (Frequency >= 245) { //245-31250 Hz
				N = 128;
				Prescaler = _BV(CS22) | _BV(CS20);
			}
			else if (Frequency >= 123) { //123-15625 Hz
				N = 256;
				Prescaler = _BV(CS22) | _BV(CS21);
			}
			else if (Frequency >= 31) { //31-3906 Hz
				N = 1024;
				Prescaler = _BV(CS22) | _BV(CS21) | _BV(CS20);
			}
			else {
				return (1); //invalid frequency
			}
			TCCR2B = _BV(WGM22) | Prescaler;
			OCR2A = round(16000000 / (2 * (float)(N*Frequency))) - 1; //OCR2A sets frequency
			OCR2B = Setpoint;
			break;
		}

	case 2:
		//	          ------          ------
		//	|	     |      |        |
		//	| 	     |      |        |
		//	 --------        --------                 OC2A f=Frequency
		//
		//	  ------          ------          ------
		//	        |        |      |        |
		//	        |        |      |        |
		//	         --------        --------         OC2B f=Frequency
		//WGM22   WGM21  WGM20
		//   0      0      1  Phase correct PWM, Top=0xff, Update of UCR2B at Top, TOV Flag set on Top
		//
		//COM2A1 COM2A0
		//   1      1    Set OC2A on Compare Match when up-counting. Clear OC2A on Compare Match when down - counting (non-inverting mode)
		//
		//COM2B1 COM2B0
		//   1      0    Clear OC2B on Compare Match when up-counting. Set OC2B on Compare Match when down - counting(non-inverting mode)

		//PWM Freq=16000000/(N*510)
		//frequencies (0<=OCR2B<=127): 31373, 3922, 980, 490, 245, 123, 31 Hz
		{
			switch (Frequency)
			{
			case 31373:
				N = 1;
				Prescaler = _BV(CS20);
				break;
			case 3922:
				N = 8;
				Prescaler = _BV(CS21);
				break;
			case 980:
				N = 32;
				Prescaler = _BV(CS21) | _BV(CS20);
				break;
			case 490:
				N = 64;
				Prescaler = _BV(CS22);
				break;
			case 245:
				N = 128;
				Prescaler = _BV(CS22) | _BV(CS20);
				break;
			case 123:
				N = 256;
				Prescaler = _BV(CS22) | _BV(CS21);
				break;
			case 31:
				N = 1024;
				Prescaler = _BV(CS22) | _BV(CS21) | _BV(CS20);
				break;
			default:
				return (1); //invalid frequency
			}
			if (!Invert) {
				TCCR2A = _BV(COM2A1) | _BV(COM2A0) | _BV(COM2B1) | _BV(WGM20);
			}
			else {
				TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(COM2B0) | _BV(WGM20);
			}
			TCCR2B = Prescaler;
			OCR2B = Setpoint;
			OCR2A = 255 - OCR2B;
			break;
		}
	}
	return 0;
}

void Timer2PWMClass::end() {
	//stop timer2
	//TCCR2A
	//COM2A1 COM2A0 COM2B1 COM2B0 ----- ----- WGM21 WGM20
	//   0      0      0      0     0     0     0     0
	//
	//TCCR2B
	//FOC2A   FOC2B  -----  ----- WGM22  CS22  CS21  CS20
	//   0      0      0      0     0     0     0     0
	//
	//WGM22   WGM21  WGM20
	//   0      0        Normal Mode of Operation
	//
	//COM2A1 COM2A0
	//   0      0    WGM22 = 0: Normal port operation, OC2B disconnected
	//
	//COM2B1 COM2B0
	//   0      0    Normal port operation, OC2B disconnected
	//
	//  CS22   CS21   CS20
	//   0      0      0   No clock source (Timer stopped)
	//
	TCCR2A = 0;
	TCCR2B = 0;
}

void Timer2PWMClass::set(byte Setpoint)
{
	switch (_Mode)
	{
		case 1:
			OCR2B = Setpoint;
			break;
		case 2:
			OCR2B = Setpoint;
			OCR2A = 255 - OCR2B;
			break;
	}
}

Timer2PWMClass Timer2PWM;
