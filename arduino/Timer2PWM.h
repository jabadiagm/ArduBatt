/*
Timer2PWM.h - Library for PWM converter.
J. Castillejo, May 7th, 2016.
Released into the public domain.
*/
#ifndef Timer2PWM_h
#define Timer2PWM_h

#include "Arduino.h"

class Timer2PWMClass
{
public:
	byte begin(byte Mode, long Frequency, bool Invert, byte Setpoint);
	void end();
	void set(byte Setpoint);
private:
	int _Mode;
};

extern Timer2PWMClass Timer2PWM;

#endif