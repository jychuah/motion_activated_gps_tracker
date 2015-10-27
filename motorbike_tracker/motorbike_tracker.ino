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

#include <Wire.h>
#include "Adafruit_FONA.h"
#include "Adafruit_MMA8451.h"
#include "RTClib.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>

/**************************************************************************************
User specific settings
***************************************************************************************/
#define		APN					"fast.t-mobile.com"
#define     UID					"d76db2b8-be35-477c-a428-2623d523fbfd"
/**************************************************************************************/
//#define DEBUG true
//#define SIMULATE true
//#define INFO true
#define SLEEP_ENABLED true

#define GPS_CHANGED  0
#define GPS_NO_CHANGE 1
#define GPS_NO_LOCK -1
#define GPS_CHANGE_THRESHOLD 0.0002f

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_MAX_ATTEMPTS 5
#define FONA_DELAY 5000
#define FONA_KEY_PIN 8
#define FONA_POWER_PIN 7

#define HELPER_URL		"http://webpersistent.com/motorbike-tracker/helper/post.php"


// Event strings
const char BATTERY[] PROGMEM = "battery";
const char PROVISION[] PROGMEM = "provision";
const char GPS[] PROGMEM = "gps";
const char WAKE_EVENT[] PROGMEM = "wake";
const char BOOT[] PROGMEM = "boot";
const char SLEEP[] PROGMEM = "sleep";
const char TEMPERATURE[] PROGMEM = "temperature";
const char ERROR[] PROGMEM = "ERROR";
const char OK[] PROGMEM = "OK";
const char GPS_TOKEN[] PROGMEM = ",";

// Format strings
const char TIMESTAMP_FORMAT[] PROGMEM = "%lu";
const char BATTERY_FORMAT[] PROGMEM = "%u";

//JSON post data indexes and lengths
#define UID_INDEX 11
#define IMEI_INDEX 60
#define SEQUENCE_INDEX 92
#define LOCATION_INDEX 129
#define TIMESTAMP_INDEX 171
#define TYPE_INDEX 194
#define DATA_INDEX 223

#define UID_LENGTH 36
#define IMEI_LENGTH 15
#define SEQUENCE_LENGTH 20
#define LOCATION_LENGTH 24
#define TIMESTAMP_LENGTH 10
#define TYPE_LENGTH 17
#define DATA_LENGTH 40

//JSON post buffer
char postdata[] = "{ \"uid\" : \"axxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxz\", "
					"\"imei\" : \"012345678901234\", "
					"\"sequence\" : \"-abcdefghijklmnopqrs\", "
					"\"location\" : \"-180.123456, -180.123456\", "
					"\"timestamp\" : \"SERVER    \", "
					"\"type\" : \"event_type_string\","
					"\"data\" : \"0123456789012345678901234567890123456789\" }";

#define BUFFER_LENGTH 120
char buffer[BUFFER_LENGTH];
float lat = 0, lng = 0;

/*******************************************************************

Utility methods

********************************************************************/
#define clearBuffer()		memset(buffer, 0, BUFFER_LENGTH)

bool attempt(bool (*callback)()) {
	int attempts = 0;
	while (!callback() && attempts < FONA_MAX_ATTEMPTS) {
		delay(FONA_DELAY + attempts * 1000);
	}
	return attempts < FONA_MAX_ATTEMPTS;
}

bool attempt(int(*callback)(), int result) {
	int attempts = 0;
	while (callback() != result && attempts < FONA_MAX_ATTEMPTS) {
		delay(FONA_DELAY + attempts * 1000);
	}
	return attempts < FONA_MAX_ATTEMPTS;
}

void debug(const __FlashStringHelper* message) {
#ifdef DEBUG
	Serial.println(message);
#endif
}

void debug(const char* message) {
#ifdef DEBUG
	Serial.println(message);
#endif
}

void info(const __FlashStringHelper* message) {
#ifdef INFO
	Serial.println(message);
#endif
}


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

// Hardware definitions
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
Adafruit_MMA8451 mma = Adafruit_MMA8451();

bool accelerometer_present = false;
bool fona_present = false;
bool hardware_rtc_present = false;
DateTime current_time = DateTime(1970, 1, 1, 0, 0, 0);
volatile int sleep_cycles = 0;
volatile int f_wdt = 1;

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;
volatile bool accelerometer_interrupt = false;
volatile bool wake_timer_expired = false;
volatile bool should_sleep = true;
int wake_rate = 1;

/*****************************************************************

Setup and Loop

******************************************************************/

void setup() {
	pinMode(FONA_KEY_PIN, OUTPUT);
	pinMode(FONA_POWER_PIN, INPUT);
	digitalWrite(FONA_KEY_PIN, LOW);
	clearPostData(UID_INDEX, UID_LENGTH);
	clearPostData(TYPE_INDEX, TYPE_LENGTH);
	clearPostData(IMEI_INDEX, IMEI_LENGTH);
	clearPostData(SEQUENCE_INDEX, SEQUENCE_LENGTH);
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	clearPostData(DATA_INDEX, DATA_LENGTH);
	while (!Serial);
	Serial.begin(9600);

	while (!fonaInit());									// FONA must init!
	accelerometer_present = accelerometerInit();

	uint16_t chargeStatus = -1;
	fona.getBattChargeStatus(&chargeStatus);
	if (chargeStatus == 0) {
		info(F("Fona battery is Charging"));
	}
	attempt(&getGPS, GPS_CHANGED);
	attempt(&logBoot);
#ifdef SLEEP_ENABLED
	initWDT();
	setSleep();
#endif
}

void loop() {
#ifdef SLEEP_ENABLED
	doSleepTimer();
#endif
	if (accelerometer_present && accelerometer_interrupt) {
		info(F("Accelerometer interrupt!!!!"));
	}
#ifndef SLEEP_ENABLED
	wake_timer_expired = true;
#endif
	if (wake_timer_expired) {
		info(F("Wake timer expired."));
		attempt(&logGPS);
//		logBattery();
		setSleep();
	}
#ifndef SLEEP_ENABLED
	delay(8000);
#endif
}

/*******************************************************************

Hardware Init Functions

********************************************************************/

bool fonaInit() {
	info(F("FONA basic test"));
	info(F("Initializing....(May take 3 seconds)"));

	fonaSerial->begin(9600);

	if (!fona.begin(*fonaSerial)) {
		info(F("Couldn't find FONA"));
		return false;
	}

	type = fona.type();

	// Print SIM card IMEI number.
	clearBuffer();
	uint8_t imeiLen = fona.getIMEI(buffer);

	// Set IMEI in post data
	strncpy(&postdata[IMEI_INDEX], buffer, 15);

	// Set UID in post data
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	fona.setGPRSNetworkSettings(F(APN));

	bool result = false;

	info(F("Enabling GPRS"));
	result = attempt(&fona_enable_gprs);
	if (!result) return false;
	
	info(F("Enabling GPS"));
	result = attempt(&fona_enable_gps);
	if (!result) return false;

	fona.enableRTC(true);
	fona.enableNTPTimeSync(true, NULL);

	info(F("Fona Init Successful"));
	return true;
}

bool fona_enable_gprs() {
	return fona.enableGPRS(true);
}

bool fona_enable_gps() {
	return fona.enableGPS(true);
}

bool fona_powered_up() {
	return (digitalRead(FONA_POWER_PIN) == HIGH);
}

bool fona_key() {
	info(F("Attempting key"));
	digitalWrite(FONA_KEY_PIN, LOW);
	delay(FONA_DELAY);
	digitalWrite(FONA_KEY_PIN, HIGH);
	delay(FONA_DELAY);
	info(F("key attempt finished"));
	return fona_powered_up();
}

bool fonaRestart() {
	if (!attempt(&fona_key)) return false;

	// Fona MUST reinitialize
	while (!fonaInit()) { 
		info(F("Couldn't re-initialize Fona!"));
		delay(1000); 
	}
	return true;
}

bool accelerometerInit() {
	if (!mma.begin()) {
		info(F("Coudln't initialize MMA8451 Accelerometer"));
		return false;
	}
	else {
		info(F("MMA8451 Accelerometer OK"));
		return true;
	}
}

/*******************************************************************

Sleep Functions

********************************************************************/

void doSleepTimer() {
	if (sleep_cycles < (wake_rate * 16 / 8) && should_sleep) {
		sleep_cycles++;
		enterSleep();
	}
	if (sleep_cycles >= (wake_rate * 16 / 8) && should_sleep) {
		// sleep expired
		Serial.begin(9600);
		info(F("Sleep expired"));
		should_sleep = false;
		wake_timer_expired = true;
		fonaRestart();
	}
	if (accelerometer_present && mma.motionDetected() && should_sleep) {				// detect motion and clear latch
		Serial.begin(9600);
		info(F("Motion detected"));
		should_sleep = false;
		accelerometer_interrupt = true;
		fonaRestart();
	}
}

void setSleep() {
	mma.motionDetected();
	accelerometer_interrupt = false;
	wake_timer_expired = false;
	sleep_cycles = 0;
	should_sleep = true;
}

void initWDT() {
	info(F("Initializing Watchdog Timer"));
	delay(1000);
	/* Clear the reset flag. */
	MCUSR &= ~(1 << WDRF);

	/* In order to change WDE or the prescaler, we need to
	* set WDCE (This will allow updates for 4 clock cycles).
	*/
	WDTCSR |= (1 << WDCE) | (1 << WDE);

	/* set new watchdog timeout prescaler value */
	WDTCSR = 1 << WDP0 | 1 << WDP3; /* 8.0 seconds */

	/* Enable the WD interrupt (note no reset). */
	WDTCSR |= _BV(WDIE);
}

ISR(WDT_vect)
{
}

void enterSleep(void)
{
	if (fona_powered_up()) {
		Serial.begin(9600);
		info(F("Powering down FONA"));
		digitalWrite(FONA_KEY_PIN, HIGH);
		fona.powerDown();
		delay(500);
		Serial.end();
	}
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);   
	sleep_enable();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_bod_disable();
	sei();
	sleep_cpu();
	/* Now enter sleep mode. */
	sleep_mode();
	/* The program will continue from here after the WDT timeout*/
	sleep_disable(); /* First thing to do is disable sleep. */
	/* Re-enable the peripherals. */
	power_all_enable();
}


/*******************************************************************

Post Functions

********************************************************************/
bool postError(void) {
	return strncmp_P(buffer, ERROR, strlen(ERROR)) == 0;
}
bool postOK(void) {
	return strncmp_P(buffer, OK, strlen(OK)) == 0;
}

void setPostData(char *data, int index) {
	int length = strlen(data);
	if (data[strlen(data)] == 0) {
		length = length - 1;
	}
	strncpy(&postdata[index], data, strlen(data));
}

void clearPostData(int index, int length) {
	memset(&postdata[index], ' ', length);
}

void setEvent(const char *event) {
	clearPostData(TYPE_INDEX, TYPE_LENGTH); 
	strncpy_P(&postdata[TYPE_INDEX], event, strlen_P(event));
}

bool sendPostData() {
	clearBuffer();

	uint16_t statuscode;
	int16_t length;
	if (!fona.HTTP_POST_start(HELPER_URL, F("application/json"), (uint8_t *)postdata, strlen(postdata), &statuscode, (uint16_t *)&length)) {
		return false;
	}
	int i = 0;
	while (length > 0) {
		while (fona.available()) {
			char c = fona.read();
			buffer[i] = c;
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
			loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty.
			UDR0 = c;
#else
			Serial.write(c);
#endif
			length--;
			i++;
			if (!length || i >= BUFFER_LENGTH) break;
		}
	}
	Serial.println(F("\n"));
	fona.HTTP_POST_end();
	return true;
}


/*******************************************************************

Tracker Event Functions

********************************************************************/

bool setTimeStamp() {
	return true;
	char num[4] = { 0 };
	if (hardware_rtc_present) {
		// get hardware RTC value
		return true;
	}
	else {
		if (fona_present) {
			clearBuffer();
			fona.getTime(buffer, BUFFER_LENGTH);
			num[0] = buffer[1];
			num[1] = buffer[2];
			int year = atoi(num) + 2000;
			num[0] = buffer[4];
			num[1] = buffer[5];
			int month = atoi(num);
			num[0] = buffer[7];
			num[1] = buffer[8];
			int day = atoi(num);
			num[0] = buffer[10];
			num[1] = buffer[11];
			int hour = atoi(num);
			num[0] = buffer[13];
			num[1] = buffer[14];
			int minute = atoi(num);
			num[0] = buffer[16];
			num[1] = buffer[17];
			int second = atoi(num);

			current_time.setTime(year, month, day, hour, minute, second);
			clearBuffer();
			sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
			setPostData(buffer, TIMESTAMP_INDEX);

			debug(postdata);

			return true;
		}
		else {
			// No timestamp available
			return false;
		}
	}
	return false;
}

bool logWake() {
	setTimeStamp();
	setEvent(WAKE_EVENT);
	debug("Wake postdata");
	debug(postdata);
#ifndef SIMULATE
	if (!sendPostData()) return false;
#endif
#ifdef SIMULATE
	return true;
#endif
	return postError();
}


bool logBoot() {
	int attempts = 0;
	info(F("Starting sequence"));
	clearBuffer();
	sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
	setPostData(buffer, SEQUENCE_INDEX);
	setEvent(BOOT);
#ifndef SIMULATE
	if (!sendPostData()) return false;
#endif

#ifdef SIMULATE
	return true;
#endif
	if (postError()) {
		return false;
	}

	return true;
}


bool logBattery() {
	setTimeStamp();
	uint16_t vbat;
	if (fona.getBattPercent(&vbat)) {
		if (vbat > 0) {
			setEvent(BATTERY);
			clearBuffer();
			sprintf_P(buffer, BATTERY_FORMAT, vbat);
			clearPostData(DATA_INDEX, DATA_LENGTH);
			setPostData(buffer, DATA_INDEX);
			debug(postdata);
#ifndef SIMULATE
			if (!sendPostData()) return false;
#endif

#ifdef SIMULATE
			return true;
#endif
			if (postError()) return false;
		}
	}
	else {
		info(F("Failed to read battery"));
		return false;
	}
	return true;
}

bool logGPS() {
	int result = getGPS();
	if (result == GPS_NO_CHANGE) {
		info(F("No GPS location change"));
		return true;
	}
	if (result == GPS_NO_LOCK) {
		info(F("No GPS lock"));
		return false;
	}
	info(F("GPS change"));
	setEvent(GPS);

#ifndef SIMULATE
	if (!sendPostData()) return false;
#endif

#ifdef SIMULATE
	return true;
#endif
	if (postError()) {
		return false;
	}
	return true;
}

int getGPS() {
	clearBuffer();

	char num[16] = { 0 };
	fona.getGPS(32, buffer, BUFFER_LENGTH);

	char * token = strtok_P(buffer, GPS_TOKEN);
	if (token[0] == '0') {
		info(F("No GPS lock"));
		return GPS_NO_LOCK;
	}
	token = strtok_P(NULL, GPS_TOKEN);		// set next token to lock flag
	int lockFlag = atoi(token);

	char * date = strtok_P(NULL, GPS_TOKEN);		// we are now at date
	num[0] = date[0];
	num[1] = date[1];
	num[2] = date[2];
	num[3] = date[3];
	int year = atoi(num);
	num[2] = 0;
	num[3] = 0;
	num[0] = date[4];
	num[1] = date[5];
	int month = atoi(num);
	num[0] = date[6];
	num[1] = date[7];
	int day = atoi(num);
	num[0] = date[8];
	num[1] = date[9];
	int hour = atoi(num);
	num[0] = date[10];
	num[1] = date[11];
	int minute = atoi(num);
	num[0] = date[12];
	num[1] = date[13];
	int second = atoi(num);

	current_time.setTime(year, month, day, hour, minute, second);

	if (lockFlag == 0) {
		info(F("No GPS lock"));
		return GPS_NO_LOCK;
	}
	char * latitude = strtok_P(NULL, GPS_TOKEN);		// latitude
	float newLat = atof(latitude);
	char * longitude = strtok_P(NULL, GPS_TOKEN);		// longitude
	float newLng = atof(longitude);
	char * altitude = strtok_P(NULL, GPS_TOKEN);		// altitude
	char * speed = strtok_P(NULL, GPS_TOKEN);		// speed (?)
	char * heading = strtok_P(NULL, GPS_TOKEN);		// heading (?)

	float latDiff = lat - newLat;
	float lngDiff = lng - newLng;
	if (latDiff < -1) latDiff = latDiff * -1;
	if (lngDiff < -1) lngDiff = lngDiff * -1;
	if (latDiff < GPS_CHANGE_THRESHOLD && lngDiff < GPS_CHANGE_THRESHOLD) {
		clearBuffer();
		sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
		setPostData(buffer, TIMESTAMP_INDEX);
		return GPS_NO_CHANGE;
	}
	lat = newLat;
	lng = newLng;
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	clearPostData(DATA_INDEX, DATA_LENGTH);
	setPostData(latitude, LOCATION_INDEX);
	postdata[LOCATION_INDEX + strlen(latitude)] = ',';
	int dataIndex = LOCATION_INDEX + strlen(latitude) + 1;
	setPostData(longitude, dataIndex);
	setPostData(altitude, DATA_INDEX);
	dataIndex = DATA_INDEX + strlen(altitude);
	postdata[dataIndex] = ',';
	dataIndex++;
	setPostData(speed, dataIndex);
	dataIndex += strlen(speed);
	postdata[dataIndex] = ',';
	dataIndex++;
	setPostData(heading, dataIndex);

	clearBuffer();
	sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
	setPostData(buffer, TIMESTAMP_INDEX);

#ifdef DEBUG
	debug(postdata);
#endif

	return GPS_CHANGED;
}

void flushSerial() {
	while (Serial.available())
		Serial.read();
}

