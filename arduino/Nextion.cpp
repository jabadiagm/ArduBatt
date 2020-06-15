/*
  Nextion.cpp - Library for communication with Nextion Screen
  Created by J. Castillejo, June 2016.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Nextion.h"

Nextion::Nextion(HardwareSerial *SerialPort, NextionEvent *event) {
	echo = false;
	NextionSerial = SerialPort;
	_event = event;
	instructionComplete = false;
}
byte Nextion::begin(unsigned long baud) {
	NextionSerial->begin(baud);
}

byte Nextion::end() {
	NextionSerial->end();
}

byte Nextion::sendCommand(String &command) {
	//execute instruction & get return code. 0=success
	Serial.println(command);
	byte value;
	NextionSerial->print(command);
	sendEnd();
	echo_print("Command:");
	echo_println(command);
	value = readInstruction();
	if (value != 0) return (value); //timeout
	value = getSendCommandReturnCode();
	if (value != 0) return (value); //error code
}

byte Nextion::sendCommand(char *command) {
	//execute instruction & get return code. 0=success
	byte value;
	NextionSerial->print(command);
	sendEnd();
	echo_print("Command:");
	echo_println(command);
	value = readInstruction();
	if (value != 0) return (value); //timeout
	value = getSendCommandReturnCode();
	if (value != 0) return (value); //error code
}

byte Nextion::sendCommand(const __FlashStringHelper *command) {
	//execute instruction & get return code. 0=success
	byte value;
	NextionSerial->print(command);
	sendEnd();
	echo_print("Command:");
	echo_println(command);
	value = readInstruction();
	if (value != 0) return (value); //timeout
	value = getSendCommandReturnCode();
	if (value != 0) return (value); //error code
}

byte Nextion::sendCommand(const char *command) {
	//execute instruction & get return code. 0=success
	byte value;
	NextionSerial->print(command);
	sendEnd();
	echo_print("Command:");
	echo_println(command);
	value = readInstruction();
	if (value != 0) return (value); //timeout
	value = getSendCommandReturnCode();
	if (value != 0) return (value); //error code
}

byte Nextion::sendCommand(String &command, String &response) {
	//execute instruction and get return code & hex string answer
	byte value;
	String nose;
	value = sendCommand(command);
	if (value == 0) { //command executed. get response
		nose = string2HexString(instruction, instructionSize - 3);
		response = string2HexString(instruction, instructionSize - 3);
	}
}

byte Nextion::getSendCommandReturnCode() {
	//check if instruction is valid. if not, returns corresponding error code
	byte value;
	value = instruction[0]; //get fisrt byte to identify response
	if ((value >= 3 && value <= 0x1f) || value == 0) { //invalid request
		if (value == 0) value = 1;
		echo_println(String("Invalid Response"));
		return (value);

	}
	else return (0);
}

void Nextion::checkEvent() {
	//chesk if there are unread data in serial port. if so, receive instruction
	//and launch event
	byte value;
	if (NextionSerial->available()) { //data available
		value = readInstruction(); //get full instruction
		if (value == 0) { //if instruction received succesfully
			char hexInstr[2*NEXTIONINSTRUCTIONSIZE];
			hex_string2string(instruction, instructionSize - 3, hexInstr);
			hexInstr[(instructionSize - 3) * 2] = 0; //end of string
			_event->Event(hexInstr);
		}
	}
}

void Nextion::sendEnd() {
	char end[3] = { 255,255,255 };
	NextionSerial->write(end, 3);
}

byte Nextion::readInstruction() {
	//read serial data till end signal (ff ff ff) or timeout
	long startTime;
	int counter;
	int incomingByte = 0;
	bool timeOut = false;
	String nose;
	//clear previous data
	for (counter = 0; counter < NEXTIONINSTRUCTIONSIZE; counter++) instruction[counter] = 0;
	instructionSize = 0;
	instructionComplete = false;
	startTime = millis();
	//loop while pending data within timeout
	while (!instructionComplete && !timeOut) {
		if ((millis() - startTime) > NEXTIONTIMEOUT) timeOut = true;
		while (NextionSerial->available() > 0) {
			incomingByte = NextionSerial->read();
			//Serial.print("Byte:");
			//Serial.println(incomingByte);
			parse((byte)incomingByte);
			if (instructionComplete) break;
		}
	}
	if (timeOut) {
		echo_println("TimeOut");
		return 0xff; //timeout error
	}
	echo_print(String("Instruction="));
	nose = string2HexString(instruction, instructionSize);
	echo_println(nose);
	return 0;
}

void Nextion::parse(byte incomingByte) {
	//add the byte to the intruction buffer
	instruction[instructionSize] = incomingByte;
	instructionSize++;
	//check if end of instruction
	if (instruction[instructionSize - 1] == 0xff && instruction[instructionSize - 2] == 0xff && instruction[instructionSize - 3] == 0xff) {
		//removes the ff's to obtain an asciiz string
		instruction[instructionSize - 1] = 0;
		instruction[instructionSize - 2] = 0;
		instruction[instructionSize - 3] = 0;
		instructionComplete = true;
	}
}

void Nextion::echo_print(String line) {
	if (echo) Serial.print(line);
}
void Nextion::echo_println(String line) {
	if (echo) Serial.println(line);
}