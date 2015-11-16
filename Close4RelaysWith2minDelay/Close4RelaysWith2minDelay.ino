/*
 Name:		Close4RelaysWith2minDelay.ino
 Created:	2015-11-15 15:11:02
 Author:	ericdes
*/
#include "avr/sleep.h"
#include "Timer.h"
//boolean TESTING = true; // We shorten the delay to close the relays.
boolean TESTING = false;
int DELAY_TO_CLOSE = 120; // The delay in seconds.

/* 
 - When no voltage is detected (value: LOW) at this pin,
 the 4 relays must close after a delay of 2 minutes,
 then the arduino must go into sleep mode.
 
 - When voltage is detected (value HIGH) at this pin,
 we must waken up the arduino (if necessary),
 then immediately open the 4 relays (no delay).
*/
int VOLTAGE_PIN = 2;
int RELAY_PINS[4] = { 4, 6, 8, 10 }; // The output pins to the 4 relays.

volatile int voltageState = LOW;
volatile int closeTimerTicks = 0;
volatile int blinkTimes = 0;

Timer *DebounceTimer = new Timer(100, &OpenOrCloseRelays, true); // 100ms
Timer *CloseTimer = new Timer(5000, &CloseAllRelaysThenSleep);
Timer *BlinkTimer = new Timer(50, &Blink);
Timer *BlinkCloseTimer = new Timer(600, &BlinkWhileWaitingToClose);
Timer *PutToSleepTimer = new Timer(1000, &SleepNow, true);

// Will blink if a change of volatage state occurs.
void Blink()
{
	blinkTimes++;
	if (blinkTimes > 7)
	{
		BlinkTimer->Stop();
		blinkTimes = 0;
		digitalWrite(LED_BUILTIN, LOW);
		return;
	}
	volatile int ledState = digitalRead(LED_BUILTIN);
	if (ledState == HIGH)
	{
		digitalWrite(LED_BUILTIN, LOW);
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}
}

// Will blink while closing time is running.
void BlinkWhileWaitingToClose()
{
	if (CloseTimer->isEnabled() == false)
	{
		BlinkCloseTimer->Stop();
		digitalWrite(LED_BUILTIN, LOW);
		return;
	}
	volatile int ledState = digitalRead(LED_BUILTIN);
	if (ledState == HIGH)
	{
		digitalWrite(LED_BUILTIN, LOW);
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
	}
}

int GetInputVoltage()
{
	Serial.print("voltage: ");
	int input = digitalRead(VOLTAGE_PIN);
	if (input == LOW) Serial.println("LOW");
	if (input == HIGH) Serial.println("HIGH");
	Serial.println("");
	return input;
}

// We trigger this short interval timer to confirm the voltage reading => debounced value.
void StartDebounceTimer()
{
	//if (TESTING) Serial.print("Debounce timer start requested: ");
	DebounceTimer->Start();
	//if (TESTING) Serial.println("started.");
}

// The DebounceTimer fires this function.
void OpenOrCloseRelays()
{
	Serial.print("Debounced ");
	int voltage = GetInputVoltage();
	if (voltage == LOW)
	{
		StartCloseTimer();
	}
	else
	{
		OpenAllRelays();
	}


}

// We start the CloseTimer once debouncing the input voltage has been processed.
void StartCloseTimer()
{
	//if (TESTING) Serial.print("Close timer start requested: ");
	unsigned long expectedTicks;
	if (TESTING)
	{
		expectedTicks = 1;
	}
	else
	{
		expectedTicks = DELAY_TO_CLOSE * (unsigned long)1000 / CloseTimer->getInterval();
	}
	BlinkTimer->Start();
	CloseTimer->setEnabled(true);
	CloseTimer->Start();
	//if (TESTING) Serial.println("started.");
	int toWait = expectedTicks * CloseTimer->getInterval() / (unsigned long)1000; // in seconds;
	Serial.print("Timer to desactivate relays set to "); Serial.print(toWait); Serial.println(" seconds.");
	BlinkCloseTimer->Start();
}

void CancelCloseTimer()
{
	//if (TESTING) Serial.print("Close timer stop requested: ");
	if (CloseTimer->isEnabled())
	{
		CloseTimer->Stop();
		CloseTimer->setEnabled(false);
		closeTimerTicks = 0;
		//if (TESTING) Serial.println("stopped.");
		Serial.println("Timer to desactivate relays has been cancelled.");
	}
	else
	{
		//if (TESTING) Serial.println("was already stopped.");
	}
}

void OpenRelay(int relayPin)
{
	digitalWrite(relayPin, LOW);
	if (TESTING)
	{
		Serial.print("Relay "); Serial.print(relayPin); Serial.println(" opened.");
	}
}

void CloseRelay(int relayPin)
{
	digitalWrite(relayPin, HIGH);
	if (TESTING)
	{
		Serial.print("Relay "); Serial.print(relayPin); Serial.println(" closed.");
	}
}

// Put the Arduino to sleep.
void SleepNow()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	detachInterrupt(digitalPinToInterrupt(VOLTAGE_PIN));
	attachInterrupt(digitalPinToInterrupt(VOLTAGE_PIN), WakeUpThenOpenAllRelays, HIGH);
	sleep_mode();		// Here the device is actually put to sleep!

	sleep_disable();    // First thing after waking from sleep: disabling sleep.
	detachInterrupt(digitalPinToInterrupt(VOLTAGE_PIN));
	attachInterrupt(digitalPinToInterrupt(VOLTAGE_PIN), StartDebounceTimer, CHANGE);
}

void OpenAllRelays()
{
	Serial.println("Request to activate relays.");
	CancelCloseTimer();
	BlinkTimer->Start();
	for (int i = 0; i < 4; i++)
	{
		OpenRelay(RELAY_PINS[i]);
	}
	if (TESTING) digitalWrite(LED_BUILTIN, LOW);

}
void WakeUpThenOpenAllRelays()
{
	Serial.println("Waking up...");
	OpenAllRelays();
}

void CloseAllRelaysThenSleep()
{
	closeTimerTicks++;
	unsigned long expectedTicks;
	if (TESTING)
	{
		expectedTicks = 1;
	}
	else
	{
		expectedTicks = DELAY_TO_CLOSE * (unsigned long)1000 / CloseTimer->getInterval();
	}
	int toWait = (expectedTicks - closeTimerTicks) * CloseTimer->getInterval() / (unsigned long)1000; // in seconds;
	Serial.print("Remaining time: "); Serial.print(toWait); Serial.println(" seconds.");
	if (closeTimerTicks < expectedTicks) return;

	CloseTimer->Stop();
	CloseTimer->setEnabled(false);
	closeTimerTicks = 0;

	BlinkTimer->Start();
	for (int i = 0; i < 4; i++)
	{
		CloseRelay(RELAY_PINS[i]);
	}
	if (TESTING) digitalWrite(LED_BUILTIN, HIGH);
	PutToSleepTimer->Start();
	Serial.println("Going to sleep...");
}


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	if (TESTING) Serial.println("Testing mode enabled.");
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(VOLTAGE_PIN, INPUT); // To prevent a floating input, use a pull-down resistor.
	for (int i = 0; i < 4; i++)
	{
		pinMode(RELAY_PINS[i], OUTPUT);
	}

	//// We'll use a timer to control the delay to close the timers
	//// Library timer.h from http://png-arduino-framework.readthedocs.org/timer.html
	CloseTimer->setEnabled(false);

	attachInterrupt(digitalPinToInterrupt(VOLTAGE_PIN), StartDebounceTimer, CHANGE);
}

// the loop function runs over and over again until power down or reset
void loop() {
	BlinkTimer->Update();
	BlinkCloseTimer->Update();
	CloseTimer->Update();
	DebounceTimer->Update();
	PutToSleepTimer->Update();
	if (TESTING)
	{
		int voltage = digitalRead(VOLTAGE_PIN);
		if (voltageState != voltage)
		{
			voltageState = voltage;
			if (voltageState == LOW) Serial.println("Detected voltage LOW.");
			if (voltageState == HIGH) Serial.println("Detected voltage HIGH.");
		}
	}
}
