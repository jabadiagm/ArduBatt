/*
PID.h - Library for PID control.
J. Castillejo, May 16th, 2017.
Released into the public domain.
*/

#include "Arduino.h"
#include "PID.h"

//float PID
fPID::fPID(float gainProportional, float gainIntegral, float gainDerivative, float sampleTime)
{
	//basic PID, extra items disabled
	T = sampleTime;
	kp = gainProportional;
	ki = gainIntegral;
	kd = gainDerivative/T; //T included in kp to save one floating point operation in calculations
	if (kp != 0) activeP = true;
	else activeP = false;
	if (ki != 0) activeI = true;
	else activeI = false;
	if (kd != 0) activeD = true;
	else activeD = false;
	kick = false;
	dFilter = false; //filters disabled by default
	oFilter = false; //filters disabled by default
	oLimits = false; //saturation disabled by default
	windUp = false; //anti windup protection disabled by default
	reset();

}

void fPID::reset()
//restart previous variables
{
	IPrev = 0;
	errorPrev = 0;
	errorDPrev = 0;
	DPrev = 0;
	errorIPrev = 0;
	outputSatPrev = 0;
	outputPrev = 0;
	firstStep = true;
}

void fPID::setDerivative(bool kickProtection, float freqFilterDerivative)
//kickProtection=true -> discard reference in derivative term to prevent spikes. 
//                       use if reference is constant
//freqFilterDerivative=frequency (Hz) of low-pass filter in derivative term
//                     to avoid stability problems
{
	kick = kickProtection; //if true, discard reference to avoid spikes
	fd = freqFilterDerivative;
	if (fd != 0) {
		dFilter = true;
		alphaD = 2*PI*T*fd / (2*PI*T*fd + 1); //freq->alpha
	}
}

void fPID::setOutputLimits(float outputMax, float outputMin)
//define saturation values
{
	oMax = outputMax; //output can't be over oMax
	oMin = outputMin; //output can't be under oMin
	oLimits = true;
}

void fPID::setAntiWindUp(float outputMax, float outputMin, float windUpGain)
//define saturation values and antiwindup feedback constant
//windUpGain : feedback constantto avoid windup issues
{
	setOutputLimits(outputMax, outputMin);
	kt = windUpGain;
	windUp = true;
}

void fPID::setOutputFilter(float freqOutputFilter)
//freqOutputFilter = frequency (Hz) of low-pass filter in output to limit slew rate
{
	fo = freqOutputFilter;
	alphaO = 2 * PI*T*fo / (2 * PI*T*fo + 1); //freq->alpha
	oFilter = true;
}


/*
 error      ---
-----------| P |---------------
      |     ---                |
      |                        |
      |     ---    ---   --    |
      |----| D |--| s |-|fd|--(+)
      |     ---    ---   --    |
      |                        |             -----
      |     ---         ---    |     --     |   _ |         control
       ----| I |--(+)--|1/s|--(+)---|fo|----| _/  |---------
            ---    |    ---       |  --     |     |   |
                   |              |          -----    |
                   |    ---       |                   |
                    ---| kt|-----(-)------------------
                        ---
*/

float fPID::calculate(float setPoint, float measurement)
{
	float P=0; //proportional term
	float I=0; //integral term
	float D=0; //derivative term
	float outputFilter; //output before filter
	float outputSat; //output before saturation
	float output; //output after saturation
	error = setPoint - measurement;
	if (activeP) {
		P = kp*error;
		//Serial.print("P=");
		//Serial.println(P, 10);
	}
	if (activeD) {
		if (kick) errorD = -measurement*kd;
		else errorD = error*kd;
		if (firstStep) {
			errorDPrev = errorD; //avoid spikes at startup
			firstStep = false;
		}
		DFilt = errorD - errorDPrev; //derivative calculation (T included in kd)
		if (dFilter) D = alphaD*DFilt + (1 - alphaD)*DPrev;
		else D = DFilt;
	}
	if (activeI) {	
		if (windUp) errorI = ki*error + kt*(outputPrev - outputSatPrev);
		else errorI = ki*error;
		I = IPrev + 0.5*T*(errorI + errorIPrev); //euler integration
		//Serial.print("I=");
		//Serial.println(I, 10);
	}
	outputFilter = P + I + D; //combine terms
	if (oFilter) outputSat = alphaO*outputFilter + (1 - alphaO)*outputSatPrev;
	else outputSat = outputFilter;
	if (oLimits) {
		if (outputSat > oMax) output = oMax;
		else if (outputSat < oMin) output = oMin;
		else output = outputSat;
	} else {
		output = outputSat;
	}
	//Serial.print("output=");
	//Serial.println(output, 10);
	//save values for next interval
	IPrev = I;
	errorPrev = error;
	errorDPrev = errorD;
	DPrev = D;
	errorIPrev = errorI;
	outputSatPrev = outputSat;
	outputPrev = output;
	return output;
}