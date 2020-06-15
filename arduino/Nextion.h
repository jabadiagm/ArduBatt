/*
  Sensor.h - Library for communication with Nextion Screen
  Created by J. Castillejo, June 2016.
  Released into the public domain.
*/
#ifndef Nextion_h
#define Nextion_h

#include "Arduino.h"
#include "D:\datos\Arduino\libraries\Functions\src\Functions.h"


#define NEXTIONTIMEOUT 1000 // max waiting time =1000ms
#define NEXTIONINSTRUCTIONSIZE 100 //buffer size

struct NextionEvent //function to be implemented in the event listener. credits: Chris Brand
{
	virtual void Event(char *eventChar) = 0;
};

class Nextion //screen interface
{
public:
	Nextion(HardwareSerial *SerialPort, NextionEvent *event);
	byte begin(unsigned long baud); //connect serial port
	byte end();
	byte sendCommand(String &command); //execute instruction
	byte sendCommand(char *command); //execute instruction
	byte sendCommand(const char *command);
	byte sendCommand(const __FlashStringHelper *command);
	byte sendCommand(String &command, String &response); //execute instruction
	void checkEvent(); //read serial port to find data sent by screen
private:
	void sendEnd(); //send FFFFFF
	byte readInstruction();
	void parse(byte incomingByte);
	byte getSendCommandReturnCode();
	void echo_print(String line);
	void echo_println(String line);
	HardwareSerial *NextionSerial;
	NextionEvent *_event;
	unsigned char instruction[NEXTIONINSTRUCTIONSIZE];
	int instructionSize;
	bool instructionComplete; //flag to finish processing of instruction
	bool echo;
};



#endif