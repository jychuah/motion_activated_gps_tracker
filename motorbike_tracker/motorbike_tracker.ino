/*************************************************************************************
This code is based off of example code from the Adafruit FONA Cellular Module
with credits to Limor Fried/Ladyada from Adafruit Industries.

Designed specifically to work with the Adafruit FONA Sim808
----> http://www.adafruit.com/products/1946
----> http://www.adafruit.com/products/1963
----> http://www.adafruit.com/products/2468
----> http://www.adafruit.com/products/2542

Please support Adafruit.

BSD license, all text above must be included in any redistribution
***************************************************************************************/

#include "Adafruit_FONA.h"
#include <Wire.h>
#include <avr/sleep.h>

/**************************************************************************************
User specific settings
***************************************************************************************/
#define		APN					"fast.t-mobile.com"
#define     UID					"d76db2b8-be35-477c-a428-2623d523fbfd"
#define		IMEI				"865067020757418"
/**************************************************************************************/
#define DEBUG true


/***************************** Accelerometer Definitions ******************************/
#define ACCEL_INT1_PIN							2
#define LSM303_ADDRESS_ACCEL					(0x32 >> 1)
#define LSM303_REGISTER_ACCEL_CTRL_REG1_A		0x20
#define LSM303_REGISTER_ACCEL_CTRL_REG3_A		0x22
#define LSM303_REGISTER_ACCEL_INT1_CFG_A		0x30
#define LSM303_REGISTER_ACCEL_INT1_THS_A		0x32
#define LSM303_REGISTER_ACCEL_INT1_DURATION_A	0x33
#define SENSITIVITY_PEAK_VOLTAGE				2.45


#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define HELPER_URL		"http://webpersistent.com/motorbike-tracker/helper/post.php"
#define setEvent(a)		strncpy_P(&postdata[EVENT_INDEX], a, strlen(a));
#define postError()		strncmp_P(postresult, ERROR, strlen(ERROR)) == 0



// Event strings
const char BATTERY[] PROGMEM = "Battery";
const char NEW_SEQUENCE[] PROGMEM = "NEW SEQUENCE";
const char GPS_LOCATION[] PROGMEM = "GPS Location";
const char WAKE_EVENT[] PROGMEM = "TRACKER WAKE";
const char ERROR[] PROGMEM = "ERROR";


#define UID_INDEX 11
#define IMEI_INDEX 60
#define EVENT_INDEX 89
#define DATA_INDEX 118
#define SEQUENCE_INDEX 255

char postdata[] = "{ \"uid\" : \"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\", "
"\"imei\" : \"xxxxxxxxxxxxxxx\", \"type\" : \"event_type_string\","
"\"data\" : \"012345678911234567892123456789312345678941234567895123456789612345678971234567898123456789912345678901234567891123456789\""
", \"sequence\" : \"                        \" }";

#define POST_RESULT_LENGTH 30
char postresult[POST_RESULT_LENGTH];

// This is to handle the absence of software serial on platforms
// like the Arduino Due. Modify this code if you are using different
// hardware serial port, or if you are using a non-avr platform
// that supports software serial.
#ifdef __AVR__
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
#else
HardwareSerial *fonaSerial = &Serial1;
#endif

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;

void setup() {
	while (!Serial);

	Serial.begin(4800);
	Serial.println(F("FONA basic test"));
	Serial.println(F("Initializing....(May take 3 seconds)"));

	fonaSerial->begin(4800);

	if (!fona.begin(*fonaSerial)) {
		Serial.println(F("Couldn't find FONA"));
		while (1);
	}
	type = fona.type();

	// Print SIM card IMEI number.
	char imei[15] = { 0 }; // MUST use a 16 character buffer for IMEI!
	uint8_t imeiLen = fona.getIMEI(imei);

	// Set IMEI in post data
	strncpy(&postdata[IMEI_INDEX], imei, 15);

	// Set UID in post data
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	clearPostData();

	initSensors();

	fona.setGPRSNetworkSettings(F(APN));

	Serial.println(F("Enabling GPRS"));
	while (!fona.enableGPRS(true)) delay(5000);

	Serial.println(F("Enabling GPS"));
	while (!fona.enableGPS(true)) delay(5000);

	Serial.println(F("Starting sequence"));
	while (!newSequence()) delay(5000);

	//initInterrupt();
	//logWakeEvent();
}

void loop() {
	// flush input
	flushSerial();
	while (fona.available()) {
		Serial.write(fona.read());
	}
	checkLowBattery();
	logGPSLocation();
	delay(10000);

}

void initPostData() {
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	strncpy(&postdata[IMEI_INDEX], IMEI, strlen(IMEI));
}

void clearPostData() {
	memset(&postdata[EVENT_INDEX], ' ', 17);
	memset(&postdata[DATA_INDEX], ' ', 120);
}

bool sendPostData() {
	uint16_t statuscode;
	int16_t length;

	if (!fona.HTTP_POST_start(HELPER_URL, F("application/json"), (uint8_t *)postdata, strlen(postdata), &statuscode, (uint16_t *)&length)) {
		return false;
	}
	int i = 0;
	while (length > 0) {
		while (fona.available()) {
			char c = fona.read();

			postresult[i] = c;

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
			loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty.
			UDR0 = c;
#else
			Serial.write(c);
#endif

			length--;
			i++;
			if (!length || i >= POST_RESULT_LENGTH) break;
		}
	}
	memset(&postresult[i], 0, POST_RESULT_LENGTH - i);
	Serial.println(F("\n"));
	fona.HTTP_POST_end();
	return true;
}

bool logWakeEvent() {
	clearPostData();
	setEvent(WAKE_EVENT);
	if (!sendPostData()) return false;
	return postError();
}

bool checkLowBattery() {
	uint16_t vbat;

	if (fona.getBattPercent(&vbat)) {
		if (vbat < 10) {
			clearPostData();
			setEvent(BATTERY);

			// TODO: copy battery to data...

			if (!sendPostData()) return false;
			if (postError()) return false;
		}
	}
	else {
		Serial.print(F("Failed to read battery"));
		return false;
	}
	return true;
}

bool logGPSLocation() {
	clearPostData();
	setEvent(GPS_LOCATION);
	fona.getGPS(0, &postdata[DATA_INDEX], 120);
	if (!sendPostData()) return false;
	if (postError()) {
		return false;
	}
	return true;
}

bool newSequence() {
	clearPostData();
	setEvent(NEW_SEQUENCE);
	if (!sendPostData()) return false;
	if (postError()) {
		return false;
	}
	strncpy(&postdata[SEQUENCE_INDEX], postresult, strlen(postresult));
	return true;
}

void flushSerial() {
	while (Serial.available())
		Serial.read();
}

void initInterrupt() {
	pinMode(ACCEL_INT1_PIN, INPUT);
	sleep_enable();
	attachInterrupt(digitalPinToInterrupt(ACCEL_INT1_PIN), accelerometerISR, CHANGE);
	Serial.println(F("Going to sleep"));
	delay(1000);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_bod_disable();
	sei();
	sleep_cpu();

	// wake event
	sleep_disable();
}


void accelerometerISR() {
	sleep_disable();
	detachInterrupt(digitalPinToInterrupt(ACCEL_INT1_PIN));
	Serial.println(F("Accelerometer Interrupt"));

}
void initSensors()
{
	Serial.println(F("Initializing Sensors"));
	Wire.begin();

	uint8_t cfg1_value = 0x5F;   // \100 hODR cycle, low power, 3 axis

	// Enable the accelerometer (100Hz)
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, cfg1_value);
	// AOI interrupt enable -- position interrupt enabled on LIN1
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG3_A, 0x60);
	// INT1 Configuration -- OR movement recognition, X Y Z High event
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_CFG_A, 0x2A);
	// INT1 Threshold -- absolute value of movement greater than 16 milli-Gs
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_THS_A, 0x4F);
	// INT1 Duration -- movement must be for more than 1 ODR cycle... maybe 10ms
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_DURATION_A, 0x01);


	// LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check
	// if we are connected or not
	uint8_t reg1_a = read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A);
	if (reg1_a != cfg1_value)
	{
		Serial.println(F("Error initializing LSM303"));
	}
	else {
		Serial.println(F("Done initializing LSM303"));
	}
}

void write8(byte address, byte reg, byte value)
{
	Wire.beginTransmission(address);
#if ARDUINO >= 100
	Wire.write((uint8_t)reg);
	Wire.write((uint8_t)value);
#else
	Wire.send(reg);
	Wire.send(value);
#endif
	Wire.endTransmission();
}

byte read8(byte address, byte reg)
{
	byte value;

	Wire.beginTransmission(address);
#if ARDUINO >= 100
	Wire.write((uint8_t)reg);
#else
	Wire.send(reg);
#endif
	Wire.endTransmission();
	Wire.requestFrom(address, (byte)1);
#if ARDUINO >= 100
	value = Wire.read();
#else
	value = Wire.receive();
#endif
	Wire.endTransmission();

	return value;
}
