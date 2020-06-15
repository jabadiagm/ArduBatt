/*
  Morse.cpp - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Sensor.h"

SENSOR::SENSOR(int pin, float scaleFactor, String name, float maximum,AlarmEvent *alarm, float freqFilter, float sampleTime)
{
	inputPin = pin;
	factor = scaleFactor;
	sensorName = name;
	maxValue = maximum;
	_alarm = alarm;
	Ts = sampleTime;
	output = 0;
	if (freqFilter == 0) {
		filter = false;
		//alphaFilter = 1;
	} else { //second order butterworth filter
		filter = true;
		//alphaFilter = 2 * PI*sampleTime*freqFilter / (2 * PI*sampleTime*freqFilter + 1); //
		ff = sampleTime * freqFilter;
		const double ita = 1.0 / tan(M_PI*ff);
		const double q = sqrt(2.0);
		b0 = 1.0 / (1.0 + q*ita + ita*ita);
		b1 = 2 * b0;
		b2 = b0;
		a1 = 2.0 * (ita*ita - 1.0) * b0;
		a2 = -(1.0 - q*ita + ita*ita) * b0;
		

	}
}

float SENSOR::getNew()
{
	/*const float A = 10; //sin test function
	const float f = 100;
	static float t;
	static long N = 0;
	N++;
	t = N*Ts;
	value = 10 * sin(2 * PI*f*t); */
	value = factor*analogRead(inputPin);
	//if (filter) output = alphaFilter*value + (1 - alphaFilter)*output;
	if (filter) {
		output = b0*value + b1*value_1 + b2*value_2 + a1*output_1 + a2*output_2;
		value_2 = value_1;
		value_1 = value;
		output_2 = output_1;
		output_1 = output;
	}
	else output = value;
	if (value>maxValue)
	{
		String errorMsg;
		char buffer1[10];
		char buffer2[10];
		dtostrf(value, 4, 2, buffer1);
		dtostrf(maxValue, 4, 2, buffer2);
		errorMsg = sensorName + "=" + String(buffer1) +">" + String(buffer2);
		_alarm->Alarm(errorMsg);
	}
	return output;
}
float SENSOR::getLast()
{
	return output;
}

SENSORRAW::SENSORRAW(int pin, String name, int maximum, AlarmEvent *alarm)
{
	inputPin = pin;
	sensorName = name;
	maxValue = maximum;
}

int SENSORRAW::getNew()
{
	value = analogRead(inputPin); //+0.01*value;
	if (value>maxValue)
	{
		String errorMsg;
		errorMsg = sensorName + "=" + String(value) + ">" + String(maxValue);
		_alarm->Alarm(errorMsg);
	}
	return value;
}

int SENSORRAW::getLast()
{
	return value;
}

NTC5K25::NTC5K25(int inputPin, String name, int maximum, AlarmEvent *alarm)
{
	ntcPin = inputPin; //arduino input attached to sensor
					   //analog values table
	ntc5k25AN[0] = 75;
	ntc5k25AN[1] = 171;
	ntc5k25AN[2] = 321;
	ntc5k25AN[3] = 501;
	ntc5k25AN[4] = 667;
	ntc5k25AN[5] = 792;
	ntc5k25AN[6] = 876;
	ntc5k25AN[7] = 928;
	//factors table: 20/(ntc5k25AN[k+1]-ntc5k25AN[k])
	ntc5k25Factor[0] = 0.20887;
	ntc5k25Factor[1] = 0.13268;
	ntc5k25Factor[2] = 0.11107;
	ntc5k25Factor[3] = 0.12075;
	ntc5k25Factor[4] = 0.16018;
	ntc5k25Factor[5] = 0.23808;
	ntc5k25Factor[6] = 0.38157;
	sensorName = name;
	maxValue = maximum;
	_alarm = alarm;
}

int NTC5K25::getNew()
//access sensor
//  +5V------|NTC|---|1K|-----
//                 |         _|_
//            arduino pin
//
//sensor and resistor form a voltage divider. temperature is obtained
//by interpolating in lookup table
{
	int analog; //analogRead from the input pin
	register byte counter;
	analog = analogRead(ntcPin);
	//Serial.print ("analog=");
	//Serial.println (analog);
	for (counter = 0; counter<7; counter++)
	{
		if (analog >= ntc5k25AN[counter] && analog <= ntc5k25AN[counter + 1]) //interpolation
		{
			value = (analog - ntc5k25AN[counter])*ntc5k25Factor[counter] + 20 * counter + 5;
			break;
		}
	}
	if (counter>7)
	{
		//value out of table
		if (analog<ntc5k25AN[0]) //lower than 5ºC 
		{
			value = -200;
		}
		else
		{
			value = 666; //hotter than 145 ºC 
		}
	}
	if (value>maxValue)
	{
		String errorMsg;
		errorMsg = sensorName + "=" + String(value) + ">" + String(maxValue);
		_alarm->Alarm(errorMsg);
	}
	return value;
}

int NTC5K25::getLast()
{
	return value;
}