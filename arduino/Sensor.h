/*
  Sensor.h - Library for analog signal capture & conditioning.
  Created by J. Castillejo, May 2016.
  Released into the public domain.
*/
#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"

struct AlarmEvent //function to be implemented in the event listener. credits: Chris Brand
{
	virtual void Alarm(String AlarmText) = 0;
};

class SENSOR //sensor connected to 1 analog pin
{
public:
	SENSOR(int pin, float scaleFactor, String name, float maximum, AlarmEvent *alarm,float fFilter,float sampleTime);
	float getNew(); //new measurement
	float getLast(); //last measurement
//private:
	int inputPin; //analog pin
	float factor; //scaling factor
	float Ts; //sampling time
	float value; //last measurement
	float output; //last filtered output
	String sensorName; //id string
	float maxValue; //above this limit, fire the alarm
	AlarmEvent *_alarm;
	bool filter; //true to use filter
	//float alphaFilter; //smoothing factor for low pass filter
	//second order butterword low pass filter coefficients
	float ff;
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
	float value_1;
	float value_2;
	float output_1;
	float output_2;
};

class SENSORRAW //sensor connected to 1 analog pin, no scaling, no floating point data
{
public:
	SENSORRAW(int pin, String name, int maximum, AlarmEvent *alarm);
	int getNew(); //new measurement
	int getLast(); //last measurement
private:
	int inputPin; //analog pin
	int value; //last measurement
	String sensorName; //id string
	int maxValue; //above this limit, fire the alarm
	AlarmEvent *_alarm;
};

class NTC5K25 //594-NTCLE203E3502JB0 NTC sensor with 1k pulldown resistor
{
public:
	NTC5K25(int inputPin, String name, int maximum, AlarmEvent *alarm);
	int getNew();
	int getLast();
private:
	int ntcPin;
	//analogRead table for NTC 5K@25º, for 5-145º range (5,25,45,65,85,105,125,145 ºC)
	int ntc5k25AN[8];
	float  ntc5k25Factor[7];
	int value; //last measurement
	String sensorName; //id string
	int maxValue; //above this limit, fire the alarm
	AlarmEvent *_alarm;
};

#endif