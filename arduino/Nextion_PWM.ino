#include "D:\datos\Arduino\libraries\Timer2PWM\src\Timer2PWM.h"
#include "D:\datos\Arduino\libraries\Sensor\src\Sensor.h"
#include "D:\datos\Arduino\libraries\Nextion\src\Nextion.h"
#include "D:\datos\Arduino\libraries\Functions\src\Functions.h"
#include "D:\datos\Arduino\libraries\MemoryFree\MemoryFree.h"
#include "D:\datos\Arduino\libraries\PID\src\PID.h"

class Padre
{
public:
	Padre()
	{
		Serial.println("constructor Padre");
	}
	virtual void Comun()
	{
		Serial.println("Comun Padre");
	}
	virtual void Prueba()
	{
		Comun();
	}
};

class Hijo :public Padre
{
public:
	Hijo()
	{
		Serial.println(F("constructor Hijo"));
		//Padre();
	}
	void Comun()
	{
		Serial.println("Comun Hijo");
		Padre::Comun();
	}
};

class CtNextionPWM;


void blink() {
	pinMode(13, OUTPUT);
	long counter;
	for (counter = 0; counter < 3; counter++) {
		digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(1000);              // wait for a second
		digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
		delay(1000);              // wait for a second
	}

}


bool echo = false;
//float setpoint = 0; //voltage/current reference

void bip() { Serial.println("Bip"); }

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void fastADC12_5()
//adjust prescaler to get faster conversion (12.5KHz)
{
	// set prescale to 64
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	cbi(ADCSRA, ADPS0);
}

//page objects constants
//Events
const byte EV_POWERSHORTPRESS = 1;
const byte EV_POWERLONGPRESS = 2;
const byte EV_PLUSSHORTPRESS = 3;
const byte EV_PLUSLONGPRESS = 4;
const byte EV_MINUSSHORTPRESS = 5;
const byte EV_MINUSLONGPRESS = 6;
const byte EV_SETTINGSSHORTPRESS = 7;
const byte EV_SETTINGSLONGPRESS = 8;
const byte EV_BACKSHORTPRESS = 9;
const byte EV_BACKLONGPRESS = 10;
const byte EV_VOLTAGECHECKBOX = 11;
const byte EV_CURRENTCHECKBOX = 12;
const byte EV_MANUALCHECKBOX = 13;
//Variables
const byte VAR_SENSORVI = 128;
const byte VAR_SENSORII = 129;
const byte VAR_SENSORVO = 130;
const byte VAR_SENSORIO = 131;
const byte VAR_SENSORT = 132;
const byte VAR_FSETPOINT = 133;
const byte VAR_ISETPOINT = 134;
const byte VAR_POWERMODE = 135;
const byte VAR_LOOPSTATUS = 136;



typedef enum enumLoopStatus {
	lsStop = 0,
	lsPause = 1,
	lsRun = 2,
	lsMalfunction = 3,
	lsNoState=99
} ;

typedef enum powerMode {
	pmVoltage=0,
	pmCurrent=1,
	pmManual=2
};


class iCtNextionPWM {
public:
	virtual void Init() {}
	virtual void ShowMain() {}
	virtual void ShowMainVoltage() {}
	virtual void ShowMainCurrent() {}
	virtual void ShowMainManual() {}
	virtual void ShowSettings() {}
	virtual void ShowSensors() {}
	virtual void ShowSensorsVoltage() {}
	virtual void ShowSensorsCurrent() {}
	virtual void ShowSensorsManual() {}
	virtual void ShowAlarm(char *alarmText) {}
	virtual void ProcessEvent(byte eventId) {} //button click
	virtual void DisplayVariables() {} //show variables during loop
	virtual void DisplayVariable(byte variableId) {} //show single variable
};

class config {
public:
	config();
	powerMode pwMode;
	float fSetpoint; //voltage/current reference
	int iSetpoint; //manual mode reference
	bool nextionOK; //true when screen is ready
	const float INCREMENTVOLTAGE=0.5; // +/- 0.5V
	const float INCREMENTCURRENT = 0.1; // +/- 0.1A
	const int INCREMENTMANUAL = 1; // +/-1 in pwm
	const float MAXVOLTAGE = 20; //maximum output voltage
	const float MAXCURRENT = 4; //maximum current
	const int MAXMANUAL = 127; //maximum pwm value
	const int MINMANUAL = 0; //minimum pwm value due to switching dead times
	const unsigned long TIMEADQUIREDATA = 500; //time (us) to read analog sensors
	const unsigned long TIMECONTROL = 5000; // 5000; //time (us) to control
	const unsigned long TIMEREFRESHDATA = 1000; //time (ms) to update screen data
	const int TIMESHORTPRESS = 450; //max time to keep pressing a button to take as short press
private:
}config;

config::config(){
	pwMode = pmVoltage;
	nextionOK = false;
	iSetpoint = MINMANUAL;
	fSetpoint = 0;
}


struct NextionListener : public NextionEvent
{
	using NextionEvent::Event;

	void Event(char *eventText)
	{
		static unsigned long timeButtonPress; //when a button was last pressed
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		Serial.print("Event:");
		Serial.println(eventText);
#endif
		//main screen
		//+ button
		if (strcmp_P(eventText, PSTR("65000201")) == 0) { //+ button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText,PSTR( "65000200")) == 0) { //+ button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonPlusShortPress();
				Controller->ProcessEvent(EV_PLUSSHORTPRESS);
			}
			else {
				//buttonPlusLongPress();
				Controller->ProcessEvent(EV_PLUSLONGPRESS);
			}
		}
		//- button
		if (strcmp_P(eventText, PSTR("65000301")) == 0) { //- button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65000300")) == 0) { //- button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonMinusShortPress();
				Controller->ProcessEvent(EV_MINUSSHORTPRESS);
			}
			else {
				//buttonMinusLongPress();
				Controller->ProcessEvent(EV_MINUSLONGPRESS);
			}
		}
		//power button
		if (strcmp_P(eventText, PSTR("65000501")) == 0) { //power button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65000500")) == 0) { //power button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonPowerShortPress();
				Controller->ProcessEvent(EV_POWERSHORTPRESS);
			}
			else {
				//buttonPowerLongPress();
				Controller->ProcessEvent(EV_POWERLONGPRESS);
			}
		}
		//settings button
		if (strcmp_P(eventText, PSTR("65000401")) == 0) { //settings button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65000400")) == 0) { //settings button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonSettingsShortPress();
				Controller->ProcessEvent(EV_SETTINGSSHORTPRESS);
			}
			else {
				//buttonSettingsLongPress();
				Controller->ProcessEvent(EV_SETTINGSLONGPRESS);
			}
		}
		//settings screen
		//back button
		if (strcmp_P(eventText, PSTR("65010101")) == 0) { //back button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65010100")) == 0) { //back button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonBackShortPress();
				Controller->ProcessEvent(EV_BACKSHORTPRESS);
			}
			else {
				//buttonBackLongPress();
				Controller->ProcessEvent(EV_BACKLONGPRESS);
			}
		}
		//voltage checkbox
		if (strcmp_P(eventText, PSTR("65010201")) == 0) { 
			//checkVoltagePress();
			Controller->ProcessEvent(EV_VOLTAGECHECKBOX);
		}

		//current checkbox
		if (strcmp_P(eventText,PSTR("65010301")) == 0) {
			//checkCurrentPress();
			Controller->ProcessEvent(EV_CURRENTCHECKBOX);
		}

		//manual checkbox
		if (strcmp_P(eventText, PSTR("65010401")) == 0) {
			//checkManualPress();
			Controller->ProcessEvent(EV_MANUALCHECKBOX);
		}
		//sensors screen
		//back button
		if (strcmp_P(eventText, PSTR("65020D01")) == 0) { //back button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65020D00")) == 0) { //back button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonBackShortPress();
				Controller->ProcessEvent(EV_BACKSHORTPRESS);
			}
			else {
				//buttonBackLongPress();
				Controller->ProcessEvent(EV_BACKLONGPRESS);
			}
		}
		//power button
		if (strcmp_P(eventText, PSTR("65020301")) == 0) { //power button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65020300")) == 0) { //power button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonPowerShortPress();
				Controller->ProcessEvent(EV_POWERSHORTPRESS);
			}
		}
		//+ button
		if (strcmp_P(eventText, PSTR("65020401")) == 0) { //+ button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65020400")) == 0) { //+ button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonPlusShortPress();
				Controller->ProcessEvent(EV_PLUSSHORTPRESS);
			}
			else {
				//buttonPlusLongPress();
				Controller->ProcessEvent(EV_PLUSLONGPRESS);
			}
		}
		//- button
		if (strcmp_P(eventText, PSTR("65020501")) == 0) { //- button pressed
			timeButtonPress = millis();
		}
		if (strcmp_P(eventText, PSTR("65020500")) == 0) { //- button released
			if ((millis() - timeButtonPress) < config.TIMESHORTPRESS) {
				//buttonMinusShortPress();
				Controller->ProcessEvent(EV_MINUSSHORTPRESS);
			}
			else {
				//buttonMinusLongPress();
				Controller->ProcessEvent(EV_MINUSLONGPRESS);
			}
		}
	}
	iCtNextionPWM *Controller;
} NextionListener;



/*void echo_string_serial(char *line, int n_bytes) {
	int counter;
	char output[200];
	hex_string2string(line, n_bytes, output);
	output[2 * n_bytes] = 0;
	echo_println(output);
} */
void echo_print(String line) {
	if (echo) Serial.print(line);
}
void echo_println(String line) {
	if (echo) Serial.println(line);
}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	Nextion screen(&Serial3, &NextionListener);
#else
	Nextion screen(&Serial, &NextionListener);
#endif
enumLoopStatus loopStatus = lsStop;


struct AlarmListener : public AlarmEvent
{
	using AlarmEvent::Alarm;

	void Alarm(String AlarmText)
	{
		//if the screen is working, show error in status line. if not, send by serial
		byte ret;
		char command[30]="";
		//String command;
		loopStatus = lsMalfunction; 
		Timer2PWM.end(); //stop power
		if (config.nextionOK == true) {
			AlarmText.toCharArray(command, AlarmText.length()+1,0);
			Controller->ShowAlarm(command);
			/*
			//if (config.pageNumber != 0) buttonBackShortPress(); //alarms are shown in main screen
			ret = screen.sendCommand("t2.pco=63488"); //status line to red
			if (!ret) {
				command = "t2.txt=\"" + AlarmText + "\"";
			}
			if (!ret) ret = screen.sendCommand(command);
			if (!ret) ret = screen.sendCommand("ref t2");
			
			//change power icon to red
			switch (config.pwMode) {
			case pmVoltage:
				if (!ret) ret = screen.sendCommand("b3.picc=3");
				break;
			case pmCurrent:
				if (!ret) ret = screen.sendCommand("b3.picc=7");
				break;
			case pmManual:
				if (!ret) ret = screen.sendCommand("b3.picc=11");
				break;
			}
			if (!ret) ret = screen.sendCommand("t0.txt=\"0A\""); //current to 0
			if (!ret) ret = screen.sendCommand("t1.txt=\"0V\""); //voltage to 0
			if (!ret) ret = screen.sendCommand("ref b3"); */
		}
		if (config.nextionOK == false || ret!=0) {
			byte counter;
			while (1) {
				Serial.println(AlarmText);
				for (counter = 0; counter < 50; counter++) {
					digitalWrite(13, HIGH);
					delay(25);
					digitalWrite(13, LOW);
					delay(25);
				}
			}
		}
	}
	iCtNextionPWM *Controller;
} AlarmListener;

SENSOR SensorVi(0, 0.0156, "Vi", 22, &AlarmListener,40, (float)config.TIMEADQUIREDATA / 1000000); //input voltage
SENSOR SensorIi(1, 0.0244, "Ii", 200, &AlarmListener,1, (float)config.TIMEADQUIREDATA / 1000000); //input current
SENSOR SensorVo(2, 0.018, "Vo", 20, &AlarmListener,50, (float)config.TIMEADQUIREDATA / 1000000); //output voltage
SENSOR SensorIo(3, 0.0244, "Io", 15, &AlarmListener,200,(float)config.TIMEADQUIREDATA/1000000); //output current
NTC5K25 SensorT(4, "T", 70, &AlarmListener); //Temperature
fPID ControllerI(2, 10, 0, (float)config.TIMECONTROL/1000000);
fPID ControllerV(0.01, 0.2, 0, (float)config.TIMECONTROL/1000000);

unsigned long counterAdquire=0; //number of times adquire() is called
unsigned long counterControl = 0; //number of times control() is called

class CtNextionPWM : public  iCtNextionPWM {
public:
	virtual void Init() 
	{
		strcpy(sStatus, "");
		strcpy(sSensorVi, "");
		strcpy(sSensorIi, "");
		strcpy(sSensorVo, "");
		strcpy(sSensorIo, "");
		strcpy(sSensorT, "");
		strcpy(sFSetpoint, "");
		cLoopStatus = lsNoState; //defined an imposible value to make it change on first use
	}
	virtual void ShowMain() 
	{
		byte ret = 0;
		Init();
		pageNumber = 0;
		if (!ret) ret = screen.sendCommand(F("page 0")); //go to page 0
		if (!ret) ret = screen.sendCommand(F("t2.pco=65535")); //status line to white
		DisplayLoopstatus();	
		showData(true); //reset timer
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowMain"));
	}
	virtual void ShowMainVoltage() 
	{
		byte ret = 0;
		config.pwMode = pmVoltage;
		ShowMain();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=1"));
		DisplaySensorIo();
		DisplayfSetpoint();	
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowMainVoltage"));
	}
	virtual void ShowMainCurrent() 
	{
		byte ret = 0;
		config.pwMode = pmCurrent;
		ShowMain();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=5"));
		DisplaySensorVo();
		DisplayfSetpoint();
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowMainCurrent"));
	}
	virtual void ShowMainManual()
	{
		byte ret = 0;
		config.pwMode = pmManual;
		ShowMain();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=9"));
		DisplaySensorVo();
		DisplaySensorIo();
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowMainManual"));
	}
	virtual void ShowSettings() 
	{
		byte ret = 0;
		backMode = config.pwMode; //set back mode as actual mode
		pageNumber = 1;
		if (!ret) ret = screen.sendCommand(F("page 1")); //go to page 1
		switch (config.pwMode) {
		case pmVoltage:
			ret = screen.sendCommand(F("bt0.val=1"));
			break;
		case pmCurrent:
			ret = screen.sendCommand(F("bt1.val=1"));
			break;
		case pmManual:
			ret = screen.sendCommand(F("bt2.val=1"));
			break;
		}
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowSettings"));
	}
	virtual void ShowSensors() 
	{
		byte ret = 0;
		Init();
		backMode = config.pwMode; //set back mode as actual mode
		pageNumber = 2;
		if (!ret) ret = screen.sendCommand(F("page 2")); //go to page 0
		if (!ret) ret = screen.sendCommand(F("t2.pco=65535")); //status line to white
		DisplayVariables();
		showData(true); //reset timer
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowSettings"));
	}
	virtual void ShowSensorsVoltage() 
	{
		byte ret = 0;
		config.pwMode = pmVoltage;
		ShowSensors();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=1"));
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowSensorsVoltage"));
	}
	virtual void ShowSensorsCurrent() 
	{
		byte ret = 0;
		config.pwMode = pmCurrent;
		ShowSensors();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=5"));
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowSensorsCurrent"));
	}
	virtual void ShowSensorsManual() 
	{
		byte ret = 0;
		config.pwMode = pmManual;
		ShowSensors();
		if (!ret) ret = screen.sendCommand(F("b3.picc2=9"));
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowSensorsManual"));
	}
	virtual void ShowAlarm(char *alarmText) //go to main screeen and display alarm
	{
		loopStatus = lsMalfunction;
		Timer2PWM.end(); //stop power
		byte ret = 0;
		char command[30];
		if (pageNumber != 0) { //if not in main page, go
			if (!ret) ret = screen.sendCommand(F("page 0")); //go to page 0
		}
		if (!ret) ret = screen.sendCommand("t2.pco=63488"); //status line to red
		//change power icon to red
		switch (config.pwMode) {
		case pmVoltage:
			if (!ret) ret = screen.sendCommand(F("b3.picc=3"));
			if (!ret) ret = screen.sendCommand(F("b3.picc2=1"));
			break;
		case pmCurrent:
			if (!ret) ret = screen.sendCommand(F("b3.picc=7"));
			if (!ret) ret = screen.sendCommand(F("b3.picc2=5"));
			break;
		case pmManual:
			if (!ret) ret = screen.sendCommand(F("b3.picc=11"));
			if (!ret) ret = screen.sendCommand(F("b3.picc2=9"));
			break;
		}
		if (!ret) ret = screen.sendCommand(F("ref b3"));
		strcpy_P(command, PSTR("t2.txt=\""));
		strcat(command, alarmText);
		strcat(command, "\"");
		if (!ret) ret = screen.sendCommand(command);
		if (!ret) ret = screen.sendCommand(F("ref t2"));
		NextionCheckReturnCode(ret, PSTR("CtNextionPWM/ShowAlarm"));
	}
	virtual void ProcessEvent(byte eventId) 
	{
		switch (eventId) {
		case EV_POWERSHORTPRESS:
			EventPowerShortPress();
			break;
		case EV_POWERLONGPRESS:
			EventPowerLongPress();
			break;
		case EV_PLUSSHORTPRESS:
			EventPlusPress(1);
			break;
		case EV_PLUSLONGPRESS:
			EventPlusPress(10);
			break;
		case EV_MINUSSHORTPRESS:
			EventMinusPress(1);
			break;
		case EV_MINUSLONGPRESS:
			EventMinusPress(10);
			break;
		case EV_SETTINGSSHORTPRESS:
			EventSettingsShortPress();
			break;
		case EV_BACKSHORTPRESS:
			EventBackShortPress();
			break;
		case EV_VOLTAGECHECKBOX:
			EventVoltageCheckBox();
			break;
		case EV_CURRENTCHECKBOX:
			EventCurrentCheckBox();
			break;
		case EV_MANUALCHECKBOX:
			EventManualCheckBox();
			break;
		}
	} //button click
	virtual void EventPowerShortPress() {
		//toggle power
		switch (loopStatus) {
		case lsStop:
			pause();
			break;
		case lsPause:
			run();
			break;
		case lsRun:
			pause();
			break;
		case lsMalfunction:
			loopStatus = lsPause;
			config.fSetpoint = 0;
			config.iSetpoint = config.MINMANUAL;
			switch (config.pwMode) {
			case pmVoltage:
				ShowMainVoltage();
				break;
			case pmCurrent:
				ShowMainCurrent();
				break;
			case pmManual:
				ShowMainManual();
				break;
			}
			break;
		}
		showData(true); //reset showdata timer
		DisplayLoopstatus();
	}
	virtual void EventPlusPress(int factor) {
		if (loopStatus == lsMalfunction) return; //disable button on malfunction
		switch (config.pwMode) {
		case pmVoltage:
			if ((config.fSetpoint + factor*config.INCREMENTVOLTAGE) <= config.MAXVOLTAGE) {
				config.fSetpoint += factor*config.INCREMENTVOLTAGE;
				DisplayfSetpoint();
			}
			break;
		case pmCurrent:
			if ((config.fSetpoint + factor*config.INCREMENTCURRENT) <= config.MAXCURRENT) {
				config.fSetpoint += factor*config.INCREMENTCURRENT;
				DisplayfSetpoint();
				//if (config.fSetpoint>2) AlarmListener.Alarm("Test Alarm");
			}
			break;
		case pmManual:
			if ((config.iSetpoint + factor*config.INCREMENTMANUAL) <= config.MAXMANUAL) {
				config.iSetpoint += factor*config.INCREMENTMANUAL;
				DisplayiSetpoint();
				//if (config.iSetpoint>15) AlarmListener.Alarm("Test Alarm");
			}
			break;
		}
	}
	virtual void EventMinusPress(int factor) {
		if (loopStatus == lsMalfunction) return; //disable button on malfunction
		switch (config.pwMode) {
		case pmVoltage:
			if ((config.fSetpoint - factor*config.INCREMENTVOLTAGE) >= 0) {
				config.fSetpoint -= factor*config.INCREMENTVOLTAGE;
				DisplayfSetpoint();
			}
			break;
		case pmCurrent:
			if ((config.fSetpoint - factor*config.INCREMENTCURRENT) >= 0) {
				config.fSetpoint -= factor*config.INCREMENTCURRENT;
				DisplayfSetpoint();
			}
			break;
		case pmManual:
			if ((config.iSetpoint - factor*config.INCREMENTMANUAL) >= config.MINMANUAL) {
				config.iSetpoint -= factor*config.INCREMENTMANUAL;
				DisplayiSetpoint();
			}
			break;
		}
	}
	virtual void EventSettingsShortPress()
	{
		if (loopStatus == lsMalfunction) return; //disable button on malfunction
		ShowSettings();
	}
	virtual void EventPowerLongPress()
	{
		if (loopStatus == lsMalfunction) return; //disable button on malfunction
		switch (config.pwMode) {
		case pmVoltage:
			ShowSensorsVoltage();
			break;
		case pmCurrent:
			ShowSensorsCurrent();
			break;
		case pmManual:
			ShowSensorsManual();
			break;
		}
	}
	virtual void EventVoltageCheckBox()
	{
		backMode = pmVoltage;
	}
	virtual void EventCurrentCheckBox()
	{
		backMode = pmCurrent;
	}	
	virtual void EventManualCheckBox()
	{
		backMode = pmManual;
	}
	virtual void EventBackShortPress()
	{
		//Serial.println(freeMemory());
		if (config.pwMode != backMode) { //if changing mode, pause power
			pause();
			config.fSetpoint = 0;
			config.iSetpoint = config.MINMANUAL;
		}
		switch (backMode) {
		case pmVoltage:
			ShowMainVoltage();
			break;
		case pmCurrent:
			ShowMainCurrent();
			break;
		case pmManual:
			ShowMainManual();
			break;
		}
	}
	virtual void DisplayVariables() //show variables during loop
	{
		if (pageNumber == 1) return; //only main & sensors display data
		DisplayLoopstatus();
		DisplayiSetpoint();
		if (pageNumber == 0) {
			switch (config.pwMode) {
			case pmVoltage:
				DisplaySensorIo();
				DisplayfSetpoint();
				break;
			case pmCurrent:
				DisplaySensorVo();
				DisplayfSetpoint();
				break;
			case pmManual:
				DisplaySensorVo();
				DisplaySensorIo();
				break;
			}
		}
		else { //in sensors page display all variables
			DisplaySensorVi();
			DisplaySensorIi();
			DisplaySensorVo();
			DisplaySensorIo();
			DisplaySensorT();
			DisplayfSetpoint();
		}

	} 
	virtual void DisplayVariable(byte variableId) {} //show single variable
	virtual void DisplayLoopstatus()
	{
		if (loopStatus == cLoopStatus) return;
		byte ret = 0;
		cLoopStatus = loopStatus;
		char ssStatus[20] = "";
		switch (config.pwMode) 
			{
			case pmVoltage:
				if (loopStatus == lsStop || loopStatus == lsPause) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=0"));
				}
				else if (loopStatus == lsRun) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=2"));
				}
				break;
			case pmCurrent:
				if (loopStatus == lsStop || loopStatus == lsPause) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=4"));
				}
				else if (loopStatus == lsRun) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=6"));
				}
				break;
			case pmManual:
				if (loopStatus == lsStop || loopStatus == lsPause) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=8"));
				}
				else if (loopStatus == lsRun) {
					if (!ret) ret = screen.sendCommand(F("b3.picc=10"));
				}
				break;
			}
		if (!ret) ret = screen.sendCommand("ref b3");
		
		//status line
		switch (loopStatus) {
		case lsStop:
			strcat(ssStatus, "STOP");
			break;
		case lsPause:
			strcat(ssStatus, "PAUSE");
			break;
		case lsRun:
			strcat(ssStatus, "RUN");
			break;
		}
		DisplayStatus(ssStatus);
	}
	virtual void DisplayStatus(char *status)
	{
		//put text in status line of page 0
		byte ret = 0;
		if (strcmp(sStatus, status) == 0) return;
		char command[30] = "t2.txt=\"";
		strcpy(sStatus, status);
		strcat(command, status);
		strcat(command, "\"");
		if (!ret) ret = screen.sendCommand(command);
		NextionCheckReturnCode(ret, PSTR("NextionMain/DisplayLoopstatus"));
	}
	virtual void DisplaySensorIo()
	{
		char value[6];
		setpointVoltage2string(SensorIo.getLast(), value);
		if (strcmp(sSensorIo, value) == 0) return;
		strcpy(sSensorIo, value);
		strcat(value, "A");
		switch (pageNumber) {
		case 0:
			NextionSetText(PSTR("t0.txt"), value);
			break;
		case 2:
			NextionSetText(PSTR("t4.txt"), value);
			break;
		}
	}
	virtual void DisplaySensorVo()
	{
		char value[6];
		setpointVoltage2string(SensorVo.getLast(), value);
		if (strcmp(sSensorVo, value) == 0) return;
		strcpy(sSensorVo, value);
		strcat(value, "V");
		switch (pageNumber) {
		case 0:
			NextionSetText(PSTR("t1.txt"), value);
			break;
		case 2:
			NextionSetText(PSTR("t3.txt"), value);
			break;
		}
	}
	virtual void DisplayiSetpoint()
	{
		//show setpoint in status line
		char sSetpoint[5]; //string with setpoint
		itoa(config.iSetpoint, sSetpoint, 10); //convert integer to string
		if (pageNumber == 0) {
			DisplayStatus(sSetpoint);
		}
		else {
			NextionSetText(PSTR("t7.txt"), sSetpoint);
		}
		
	}
	virtual void DisplayfSetpoint()
	{
		char value[6]="";
		if (pageNumber == 0) { //main
			switch (config.pwMode) {
			case pmVoltage:
				setpointVoltage2string(config.fSetpoint, value);
				if (strcmp(sFSetpoint, value) == 0) return;
				strcpy(sFSetpoint, value);
				strcat(value, "V");
				NextionSetText(PSTR("t1.txt"), value);
				break;
			case pmCurrent:
				setpointVoltage2string(config.fSetpoint, value);
				if (strcmp(sFSetpoint, value) == 0) return;
				strcpy(sFSetpoint, value);
				strcat(value, "A");
				NextionSetText(PSTR("t0.txt"), value);
				break;
			}
		}
		else if (pageNumber == 2) { //sensors
			switch (config.pwMode) {
			case pmVoltage:
				setpointVoltage2string(config.fSetpoint, value);
				if (strcmp(sFSetpoint, value) == 0) return;
				strcpy(sFSetpoint, value);
				strcat(value, "V");
				break;
			case pmCurrent:
				setpointVoltage2string(config.fSetpoint, value);
				if (strcmp(sFSetpoint, value) == 0) return;
				strcpy(sFSetpoint, value);
				strcat(value, "A");
				break;
			}
			NextionSetText(PSTR("t6.txt"), value);
			byte ret = 0;
			ret = screen.sendCommand(F("ref t7")); //t6 overlapps t7
			NextionCheckReturnCode(ret, PSTR("CtNextionPWM/DisplayfSetpoint"));
		}



	}
	virtual void DisplaySensorIi()
	{
		if (pageNumber != 2) return;
		char value[6];
		setpointVoltage2string(SensorIi.getLast(), value);
		if (strcmp(sSensorIi, value) == 0) return;
		strcpy(sSensorIi, value);
		strcat(value, "A");
		NextionSetText(PSTR("t1.txt"), value);
	}
	void print(char *text) {
		char command[20];
		strcpy(command, "t2.txt=\"");
		strcat(command,text);
		strcat(command, "\"");
		//if (pageNumber == 1) return; //no t2.txt in page1
		screen.sendCommand(command);
		//screen.sendCommand("ref t2");
	}
	virtual void DisplaySensorVi()
	{
		if (pageNumber != 2) return;
		char value[6];
		setpointVoltage2string(SensorVi.getLast(), value);
		if (strcmp(sSensorVi, value) == 0) return;
		strcpy(sSensorVi, value);
		strcat(value, "V");
		NextionSetText(PSTR("t0.txt"), value);
	}
	virtual void DisplaySensorT()
	{
		if (pageNumber != 2) return;
		char value[6];
		itoa((int)SensorT.getLast(), value,10);
		if (strcmp(sSensorT, value) == 0) return;
		strcpy(sSensorT, value);
		strcat(value, "C");
		NextionSetText(PSTR("t5.txt"), value);
	}
	void NextionCheckReturnCode(byte ret, char *caller)
	{
		//throws alarm if return code is not zero
		if (ret) { //error while setting power mode
			char sError[70] = "Loss of communication with nextion in ";
			config.nextionOK = false;
			strcat(sError, caller);
			AlarmListener.Alarm(String(sError));
		}
	}
	void NextionCheckReturnCode(byte ret, const char *caller)
	{
		//throws alarm if return code is not zero
		if (ret) { //error while setting power mode
			char sError[70] = "Loss of communication with nextion in ";
			config.nextionOK = false;
			strcat_P(sError, caller);
			AlarmListener.Alarm(String(sError));
		}
	}
	void setpointVoltage2string(float value, char *output) {
		//converts float setpoint to string. one decimal
		if (value<10) dtostrf(value, 3, 1, output);
		else dtostrf(value, 4, 1, output);
	}
	void setpointCurrent2string(float value, char *output) {
		//converts float setpoint to string. two decimals
		if (value<10) dtostrf(value, 4, 2, output);
		else dtostrf(value, 5, 2, output);
	}
	void NextionSetText(char *textBoxName, char *text)
	{
		//T1.txt="text"
		byte ret = 0;
		char command[30];
		strcpy(command, textBoxName);
		strcat(command, "=\"");
		strcat(command, text);
		strcat(command, "\"");
		ret = screen.sendCommand(command);
		NextionCheckReturnCode(ret, PSTR("NextionMain/NextionSetText"));
	}
	void NextionSetText(const char *textBoxName, char *text)
	{
		//T1.txt="text"
		byte ret = 0;
		char command[30];
		strcpy_P(command, textBoxName);
		strcat(command, "=\"");
		strcat(command, text);
		strcat(command, "\"");
		ret = screen.sendCommand(command);
		NextionCheckReturnCode(ret, PSTR("NextionMain/NextionSetText"));
	}
protected: //available for subclasses
	char sStatus[20];
	char sSensorVi[6];
	char sSensorIi[6];
	char sSensorVo[6];
	char sSensorIo[6];
	char sSensorT[6];
	char sFSetpoint[6];
	enumLoopStatus cLoopStatus;
	byte pageNumber; //number of screen active
	powerMode backMode; //power mode when going back to main screen
}CtNextionPWM;



void nextionInit() {
	byte ret; //return byte
	screen.begin(9600);
	ret = screen.sendCommand("bkcmd=3"); //parche 11/05/2020 para que la pantalla arranque
	delay(1000);
	ret = screen.sendCommand("bkcmd=3"); //activate echo for all requests
	//nextionGotoMainPage(); //go to page 0
	if (ret) { //communication error with nextion
		config.nextionOK = false;
		AlarmListener.Alarm("Loss of communication with nextion in nextionInit");
		loopStatus = lsMalfunction;
	}
	else {
		config.nextionOK = true;
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		Serial.println("Nextion Init OK");
#endif
	}
}





void pause() {
	//stop pwm
	Timer2PWM.set(config.MINMANUAL);
	Timer2PWM.end();
	loopStatus = lsPause;
}

void run() {
	//start pwm
	const long FREQCONTROLLER = 31373;
	const byte PWMMODE = 2;
	Timer2PWM.set(config.iSetpoint);
	Timer2PWM.begin(PWMMODE, FREQCONTROLLER, false, config.iSetpoint);
	ControllerI.reset();
	ControllerV.reset();
	showData(true); //reset internal timer
	loopStatus = lsRun;
}

void adquire() {
	//read analog sensors
	static int counter = 0; //low priority measurements
	static unsigned long timeLastAdquire = 0; //when the analog data was last read
	if ((micros() - timeLastAdquire) < config.TIMEADQUIREDATA && micros() >= timeLastAdquire) return;
	timeLastAdquire = micros();
	counterAdquire++;
	SensorVo.getNew();
	SensorIo.getNew();
	counter++;
	if (counter >= 10) { //low priority measurements are 1/10 of main measurements
		SensorVi.getNew();
		SensorIi.getNew();
		SensorT.getNew();
		counter = 0;
	}

}

void control() {
	//voltage/current feedback or manual
	static unsigned long timeLastControl = 0; //when the analog data was last read
	if ((micros() - timeLastControl) < config.TIMECONTROL && micros() >= timeLastControl) return;
	timeLastControl = micros();
	counterControl++;
	switch (config.pwMode) {
	case pmVoltage:
		float currentSetpoint;
		currentSetpoint = ControllerV.calculate(config.fSetpoint, SensorVo.getLast());
		config.iSetpoint = ControllerI.calculate(currentSetpoint, SensorIo.getLast());
		if (SensorVo.getLast() > 1.3*config.fSetpoint) config.iSetpoint = config.MINMANUAL;
		break;
	case pmCurrent:
		config.iSetpoint = ControllerI.calculate(config.fSetpoint, SensorIo.getLast());
		break;
	case pmManual:
		
		break;
	}
	Timer2PWM.set(config.iSetpoint);
}


void showData(bool reset)
{
	//update screen with sensor data. if reset=true, set timeLastShow
	static unsigned long timeLastShow = 0; //when the screen was last updated
	if (reset) {
		timeLastShow = millis(); //first pass
		return;
	}
	if ((millis() - timeLastShow) < config.TIMEREFRESHDATA && millis() >= timeLastShow) return;
	timeLastShow = millis();
	//config.iSetpoint++;
	CtNextionPWM.DisplayVariables();
}

void showSerialData()
{
	//send sensor data to serial. arduino mega only
	static unsigned long timeLastShow = 0; //when the screen was last updated
	if ((millis() - timeLastShow) < config.TIMEREFRESHDATA && millis() >= timeLastShow) return;
	timeLastShow = millis();
	Serial.print("Vi=");
	Serial.println(SensorVi.getLast());
	//Serial.print("Ii=");
	//Serial.println(SensorIi.getLast());
	Serial.print("Vo=");
	Serial.println(SensorVo.getLast());
	Serial.print("Io=");
	Serial.println(SensorIo.getLast());
	Serial.print("T=");
	Serial.println(SensorT.getLast());
	Serial.print("Set=");
	Serial.println(config.fSetpoint);
	Serial.print("PWM=");
	Serial.println(config.iSetpoint);
	Serial.println("--------");
}

void stats()
{
	static unsigned long timeLastSet = 0;
	static unsigned long counter = 0;

	counter++;
	if ((millis() - timeLastSet) < 1000 && millis() >= timeLastSet) return;
	char nose[5]; //string with setpoint
	itoa(counterControl, nose, 10); //convert integer to string
	//Serial.println(counter);
	//Serial.print("counterAdquire=");
	//Serial.println(counterAdquire);
	//Serial.print("counterControl=");
	//Serial.println(counterControl);
	//CtNextionPWM.print(String(config.iSetpoint));
	//Serial.println (nose);
	counter = 0;
	counterAdquire = 0;
	counterControl = 0;
	timeLastSet = millis();
}

void setup()
{

  /* add setup code here */
	delay(4000);
	fastADC12_5();
	const long FREQCONTROLLER = 31373;
	const byte PWMMODE = 2;
	NextionListener.Controller = &CtNextionPWM;
	AlarmListener.Controller = &CtNextionPWM;
	//ControllerI.setDerivative(false, 1000);
	//ControllerI.setOutputFilter(500);
	ControllerI.setAntiWindUp(config.MAXMANUAL,config.MINMANUAL,10);
	ControllerV.setAntiWindUp(config.MAXCURRENT,0,40);
	//ControllerV.setDerivative(true, 10);
	//pinMode(A0, INPUT);
	//pinMode(A1, INPUT);
	//Timer2PWM.begin(PWMMODE, FREQCONTROLLER, false, config.iSetpoint);
	
}


void loop()
{
	String command;
	String response;
	

	/*long counter;
	Serial.begin(38400);
	config.pwMode = pmVoltage;
	config.fSetpoint = 5;
	run();
	for (counter = 0; counter < 500000; counter++) {

		adquire();
		control();
		showSerialData();
		stats();
		//Serial.println(SensorIo.getLast());
		//delay(50);
	}
	pause();
	while (1); */
	
	unsigned long timeLastSet = 0;
	while (true) {
		switch (loopStatus) {
			case lsStop :
				//initialization
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
				Serial.begin(38400); //init serial
#endif
				nextionInit(); //init screen
				if (config.nextionOK) loopStatus = lsPause;
				CtNextionPWM.ShowMainCurrent();
				break;
			case lsPause:
				//waiting to start. sensors running
				adquire();
				showData(false);
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
				//stats();
				//showSerialData();
#endif
				break;
			case lsRun:
				//PWM+sensors running
				adquire();
				control();
				showData(false);
				//stats();
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
				
				//showSerialData();
#endif
				break;
			case lsMalfunction:

				break;
		}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
		while (Serial.available()) {
			int inByte = Serial.read();
			//Serial.print(inByte, HEX);
			if (inByte == 0xd) { //carriage return
				screen.sendCommand(command, response);
				Serial.print("Command:");
				Serial.println(command);
				Serial.print("Response:");
				Serial.println(response);
				command = "";
				response = "";
			} else command.concat(char(inByte));
		}
#endif
		
		//while ((millis() - timeLastSet) < 1 && millis() >= timeLastSet);
		//timeLastSet = millis();
		screen.checkEvent();
		//delay(500);
	}

  /* add main program code here */

}

