/*
PID.h - Library for PID control.
J. Castillejo, May 16th, 2017.
Released into the public domain.
*/
#ifndef PID_h
#define PID_h

#include "Arduino.h"

class fPID //PID using float's
{
public:
	fPID(float gainProportional,float gainIntegral, float gainDerivative, float sampleTime);
	void reset();
	void setDerivative(bool kickProtection, float freqFilterDerivative);
	void setOutputLimits(float outputMax, float outputMin);
	void setAntiWindUp(float outputMax, float outputMin, float windUpGain);
	void setOutputFilter(float freqOutputFilter);
	float calculate(float setPoint, float measurement);

private:
	float kp; //proportional gain
	float ki; //integral gain
	float kd; //derivative gain
	float T; //sample time (seconds)
	bool activeP; //compute proportional term
	bool activeI; //compute integral term
	bool activeD; //compute derivative term
	bool kick; //true if kick protection is on -> derivative is based in output only
	bool dFilter; //true if derivative filter is in use
	bool oFilter; //true if output filter is active
	bool oLimits; //true if output limits are active
	bool windUp;  //true if output limits and windup feedback are active
	bool firstStep; //avoid derivative spikes in first step
	float kt; //windup feedback gain
	float fo; //output filter frequency (Herz)
	float fd; //derivative filter frequency (Herz)
	float oMax; //maximum output
	float oMin; //minimmum output
	//
	float error; //difference between setpoint and measurement
	float errorPrev; //error in previous interval
	float errorD; //error for the derivative term
	float errorDPrev; //previous derivative error
	float DFilt; //derivative term before filter
	float DPrev; //previous derivative term
	float errorI; //error for the integral term
	float errorIPrev; //previous error for the integral term
	float IPrev; //integral term in previous interval
	float alphaD; //smoothing factor for derivative filter
	float alphaO; //smoothing factor for output filter
	float outputSatPrev; //previous output before saturation
	float outputPrev; //previous output after saturation	
};

#endif