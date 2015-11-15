/*
 Name:		Close4RelaysWith2minDelay.ino
 Created:	2015-11-15 15:11:02
 Author:	ericdes
*/
//#include <avr/io.h>
//#include <avr/interrupt.h>

/* 
 - When no voltage is detected at this pin,
 the 4 relays must close after a delay of 2 minutes,
 then the arduino must go into sleep mode.
 
 - When voltage is detected at this pin,
 we must waken up the arduino (if necessary),
 then immediately open the 4 relays (no delay).
*/
int voltagePin = 2; 

// The pins where the relays attach:
int relay1 = 4;
int relay2 = 6;
int relay3 = 8;
int relay4 = 10;

// The delay in seconds:
int delayToClose = 120;

// the setup function runs once when you press reset or power the board
void setup() {
	//pinMode(voltagePin, INPUT);
	//attachInterrupt(digitalPinToInterrupt(voltagePin), blink, CHANGE);
}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
