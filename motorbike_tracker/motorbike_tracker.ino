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


/**************************************************************************************
Debugging pre-processor definitions

**************************************************************************************/
//#define DEBUG true
//#define SIMULATE true
#define INFO true
//#define FORCE_ARM  true
//#define FORCE_CHARGE true
//#define SLEEP_DISABLED true

#define GPS_CHANGED  0
#define GPS_NO_CHANGE 1
#define GPS_NO_LOCK -1
#define GPS_CHANGE_THRESHOLD 0.0002f

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_MAX_ATTEMPTS 10
#define FONA_DELAY 5000
#define FONA_DTR_PIN 9
#define FONA_KEY_PIN 8
#define FONA_POWER_PIN 7
#define	CHARGE_DETECT_PIN A0
#define BAUD_RATE 57600
#define HELPER_URL		"http://webpersistent.com/motorbike-tracker/helper/post.php"


// Event strings
const char BATTERY[] PROGMEM = "battery";
const char GPS[] PROGMEM = "gps";
const char NOGPS[] PROGMEM = "nogps";
const char CHECKIN[] PROGMEM = "checkin";
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
#define HEADING_INDEX 226
#define BATTERY_INDEX 285
#define TEMPERATURE_INDEX 325
#define EXTRAS_INDEX 350

#define UID_LENGTH 36
#define IMEI_LENGTH 15
#define SEQUENCE_LENGTH 20
#define LOCATION_LENGTH 24
#define TIMESTAMP_LENGTH 10
#define TYPE_LENGTH 17
#define HEADING_LENGTH 32
#define BATTERY_LENGTH 20
#define	TEMPERATURE_LENGTH 10
#define EXTRAS_LENGTH 10

//JSON post buffer
char postdata[] = "{ \"uid\" : \"axxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxz\", "
					"\"imei\" : \"012345678901234\", "
					"\"sequence\" : \"-abcdefghijklmnopqrs\", "
					"\"location\" : \"-180.123456, -180.123456\", "
					"\"timestamp\" : \"SERVER    \", "
					"\"type\" : \"event_type_string\","
					"\"heading\" : \"-1234.5678,-1234.5678,-1234.5678\", "
					"\"data\" : { "
					"\"battery\" : \"b1234567890123456789\", "
					"\"temperature\" : \"t123456789\", "
					"\"extras\" : \"e123456789\" } }";

// Data buffer

#define BUFFER_LENGTH 120
char buffer[BUFFER_LENGTH];

#define MODE_START		-1
#define MODE_CHARGE		0
#define MODE_WATCHDOG	1
#define MODE_TRACKER	2

// Tracker status info

bool accelerometer_present = false;
bool fona_present = false;
bool hardware_rtc_present = false;
DateTime current_time = DateTime(1970, 1, 1, 0, 0, 0);
volatile int sleep_cycles = 0;
volatile int f_wdt = 1;
volatile int checkin_cycles = 0;
uint8_t type;
int last_gps_operation = -100;
int wake_rate = 5;
int checkin_rate = 30;
float lat = 0, lng = 0;
int chargeStatus = 0;
int battPercent = 100;
int battMilliVolts = 5000;
int stationaryCount = 0;

int mode = MODE_START;

// Tracker state flags

volatile bool accelerometer_interrupt = false;
volatile bool wake_timer_expired = false;
volatile bool notify_battery_low = false;
volatile bool notify_temperature_critical = false;
volatile bool should_sleep = true;
volatile bool alert_mode = false;

/*******************************************************************

Utility methods

********************************************************************/
#define clearBuffer()		memset(buffer, 0, BUFFER_LENGTH)

void printPostData() {
#ifdef DEBUG
	Serial.println(F("**** START POST DATA **** "));

	char buff[60] = { 0 };
	Serial.print(F("UID: "));
	strncpy(buff, &postdata[UID_INDEX], UID_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);

	Serial.print(F("IMEI: "));
	strncpy(buff, &postdata[IMEI_INDEX], IMEI_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);
	

	Serial.print(F("SEQUENCE: "));
	strncpy(buff, &postdata[SEQUENCE_INDEX], SEQUENCE_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("LOCATION: "));
	strncpy(buff, &postdata[LOCATION_INDEX], LOCATION_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("TIMESTAMP: "));
	strncpy(buff, &postdata[TIMESTAMP_INDEX], TIMESTAMP_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("TYPE: "));
	strncpy(buff, &postdata[TYPE_INDEX], TYPE_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("HEADING: "));
	strncpy(buff, &postdata[HEADING_INDEX], HEADING_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("BATTERY: "));
	strncpy(buff, &postdata[BATTERY_INDEX], BATTERY_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("TEMPERATURE: "));
	strncpy(buff, &postdata[TEMPERATURE_INDEX], TEMPERATURE_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.print(F("EXTRAS: "));
	strncpy(buff, &postdata[EXTRAS_INDEX], EXTRAS_LENGTH);
	Serial.println(buff);
	memset(buff, 0, 60);


	Serial.println(F("Postdata: "));
	Serial.println(postdata);
	Serial.println(F("**** END POST DATA ****"));
#endif
}

void flushSerial() {
	while (Serial.available())
		Serial.read();
}

void clearAllData() {
	clearPostData(UID_INDEX, UID_LENGTH);
	clearPostData(TYPE_INDEX, TYPE_LENGTH);
	clearPostData(IMEI_INDEX, IMEI_LENGTH);
	clearPostData(SEQUENCE_INDEX, SEQUENCE_LENGTH);
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	clearPostData(HEADING_INDEX, HEADING_LENGTH);
	clearPostData(BATTERY_INDEX, BATTERY_LENGTH);
	clearPostData(TEMPERATURE_INDEX, TEMPERATURE_LENGTH);
	clearPostData(EXTRAS_INDEX, EXTRAS_LENGTH);
	printPostData();
}

bool attempt(bool (*callback)()) {
	int attempts = 0;
	while (!callback() && attempts < FONA_MAX_ATTEMPTS) {
		delay(FONA_DELAY + attempts * 1000);
		attempts++;
	}
	return attempts < FONA_MAX_ATTEMPTS;
}

int attempt(int(*callback)(), int fail_result) {
	int attempts = 1;
	int callbackResult = callback();
	while (callbackResult == fail_result && attempts < FONA_MAX_ATTEMPTS) {
		delay(FONA_DELAY + attempts * 1000);
		callbackResult = callback();
		attempts++;
	}
	return callbackResult;
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
/*
void info(const __FlashStringHelper* message) {
#ifdef INFO
	Serial.println(message);
#endif
}
*/
#ifdef INFO
#define		info(a)	Serial.println(F(a));
#else
#define		info(a)	;
#endif

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


uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);



/*******************************************************************

Hardware Init Functions

********************************************************************/

bool fonaColdStart() {
	info("FONA basic test");
	info("Initializing....(May take 3 seconds)");

	fonaSerial->begin(BAUD_RATE);

	if (!fona.begin(*fonaSerial)) {
		info("Couldn't find FONA");
		return false;
	}

	type = fona.type();
	return true;
}

bool fonaInit() {
	if (!fonaColdStart()) return false;

	// Print SIM card IMEI number.
	clearBuffer();
	uint8_t imeiLen = fona.getIMEI(buffer);

	// Set IMEI in post data
	strncpy(&postdata[IMEI_INDEX], buffer, 15);

	// Set UID in post data
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	fona.setGPRSNetworkSettings(F(APN));

	fona.enableTemperatureDetection();

	bool result = false;

	info("Enabling GPRS");
	result = attempt(&fona_enable_gprs);
	if (!result) return false;
	
	info("Enabling GPS");
	result = attempt(&fona_enable_gps);
	if (!result) return false;

	fona.enableRTC(true);
	fona.enableNTPTimeSync(true, NULL);

	info("Fona Init Successful");
	return true;
}

bool fona_sleep() {
	info("Attempting FONA Sleep");
	info("Disabling GPRS"); 
	if (!fona_disable_gprs()) return false;
	info("Disabling GPS");
	if (!fona_disable_gps()) return false;
	info("Enabling Slow Clock");
	if (!fona.enableSleep(true)) return false;
	digitalWrite(FONA_DTR_PIN, HIGH);
	return true;
}

bool fona_wake() {
	digitalWrite(FONA_DTR_PIN, LOW);
	fona.enableSleep(false);
	info("Enabling GPRS");
	if (!attempt(&fona_enable_gprs)) return false;
	flushSerial();
	info("Enabling GPS");
	if (!attempt(&fona_enable_gps)) return false;
	flushSerial();
	return true;
}

bool fona_enable_gprs() {
	return fona.enableGPRS(true);
}

bool fona_disable_gprs() {
	return fona.enableGPRS(false);
}

bool fona_enable_gps() {
	return fona.enableGPS(true);
}

bool fona_disable_gps() {
	return fona.enableGPS(false);
}

bool fona_powered_up() {
	return (digitalRead(FONA_POWER_PIN) == HIGH);
}

bool fona_key() {
	info("Attempting key");
	digitalWrite(FONA_KEY_PIN, LOW);
	delay(FONA_DELAY);
	digitalWrite(FONA_KEY_PIN, HIGH);
	delay(FONA_DELAY);
	info("key attempt finished");
	return fona_powered_up();
}

bool fonaRestart() {
	if (!attempt(&fona_key)) return false;

	// Fona MUST reinitialize
	while (!fonaInit()) { 
		info("Couldn't re-initialize Fona!");
		delay(1000); 
	}
	return true;
}

bool accelerometerInit() {
	if (!mma.begin()) {
		info("Coudln't initialize MMA8451 Accelerometer");
		return false;
	}
	else {
		info("MMA8451 Accelerometer OK");
		return true;
	}
}

/*******************************************************************

Sleep Functions

********************************************************************/

void doSleepTimer() {
	Serial.begin(BAUD_RATE);
	info("Sleep timer method");
	Serial.end();
	delay(50);
	if (notify_temperature_critical && should_sleep) {
		// permanent sleep due to critical temperature;
		info("Critical temperature sleep");
		enterSleep();
	}
	else {
		if (should_sleep) {
			if (sleep_cycles < (wake_rate * 60 / 16)) {
				Serial.begin(BAUD_RATE);
				info("Sleep cycle");
				Serial.end();
				delay(50);
				sleep_cycles++;
				enterSleep();
			}
			if (sleep_cycles >= (wake_rate * 60 / 16)) {
				// sleep expired, not critical temperature
				Serial.begin(BAUD_RATE);
				info("Sleep expired");
				should_sleep = false;
				wake_timer_expired = true;
				fona_wake();
			}
			if (mode == MODE_WATCHDOG && accelerometer_present && mma.motionDetected()) {
				// detect motion and clear latch
				Serial.begin(BAUD_RATE);
				info("Motion detected");
				should_sleep = false;
				accelerometer_interrupt = true;
				alert_mode = true;
				wake_rate = 1;						// send minute updates
				fona_wake();
			}
		}
	}
}

void setSleep() {
	mma.motionDetected();
	fona_sleep();
	notify_battery_low = false;
	notify_temperature_critical = false;
	accelerometer_interrupt = false;
	wake_timer_expired = false;
	sleep_cycles = 0;
	should_sleep = true;
}

void initWDT() {
	info("Initializing Watchdog Timer");
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
#ifdef SIMULATE
	return false;
#endif
	bool result = (strncmp_P(buffer, ERROR, strlen(ERROR)) == 0);
	if (result) {
		info("Received ERROR from server!");
	}
	return result;
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
#ifdef SIMULATE
	return true;
#endif
	printPostData();
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

Tracker Event Logging Functions

********************************************************************/
bool sendToServer() {
	if (!sendPostData()) return false;
	if (!postError()) {
		readConfig();
		return true;
	}
	return false;
}

bool logBattery() {
	setEvent(BATTERY);
	info("Sending low battery notification");
	return sendToServer();
}

bool logTemperature() {
	setEvent(TEMPERATURE);
	info("Sending temperature critical notification");
	return sendToServer();
}

bool logWake() {
	setEvent(WAKE_EVENT);
	return sendToServer();
}

bool logBoot() {
	info("Starting sequence");
	bool serverTimestamp = false;
	int result = attempt(&getGPS, GPS_NO_LOCK);
	Serial.println(current_time.unixtime());
	if (current_time.unixtime() < 1444000000) {
		info("Invalid timestamp -- use server timestamp as sequence_id");
		clearPostData(SEQUENCE_INDEX, SEQUENCE_LENGTH);
		clearPostData(TIMESTAMP_INDEX, TIMESTAMP_LENGTH);
		serverTimestamp = true;
	}
	else {
		clearBuffer();
		info("Use GPS timestamp as sequence_id");
		sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
		setPostData(buffer, SEQUENCE_INDEX);
	}
	setEvent(BOOT);
	if (!sendPostData()) return false;
	if (postError()) {
		return false;
	}
	if (serverTimestamp) {
		info("Server timestamp:");
#ifdef INFO
		Serial.println(buffer);
#endif
		char* token = strtok_P(buffer, GPS_TOKEN);		// first token should be timestamp

		if (strlen(token) < 10) {
			info("Bad timestamp from server!");
			return false;
		}
		token = strtok_P(NULL, GPS_TOKEN);
		wake_rate = atoi(token);
		token = strtok_P(NULL, GPS_TOKEN);
		checkin_rate = atoi(token);
		checkRates();
		setPostData(buffer, SEQUENCE_INDEX);
		printPostData();
		return true;
	}
	readConfig();
	return true;
}

bool checkRates() {
	if (wake_rate == 0 || checkin_rate == 0) {
		info("Invalid rates");
		wake_rate = 5;
		checkin_rate = 30;
		return false;
	}
	else {
		info("Got rates");
		Serial.println(wake_rate);
		Serial.println(checkin_rate);
		return true;
	}
}

bool readConfig() {
	char* token = strtok_P(buffer, GPS_TOKEN);
	token = strtok_P(NULL, GPS_TOKEN);
	wake_rate = atoi(token);
	token = strtok_P(NULL, GPS_TOKEN);
	checkin_rate = atoi(token);
	checkRates();
}

bool logSleep() {
	setEvent(SLEEP);
	return sendToServer();
}

bool logGPS() {
	int result = attempt(&getGPS, GPS_NO_LOCK);
	if (result == last_gps_operation) {
		checkin_cycles++;
	}
	else {
		checkin_cycles = 0;
	}
	if (result == GPS_NO_LOCK) {
		info("No GPS lock");
		setEvent(NOGPS);
	}
	if (result == GPS_NO_CHANGE) {
		info("No GPS location change");
		setEvent(CHECKIN);
	}
	if (result == GPS_CHANGED) {
		info("GPS change");
		setEvent(GPS);
		checkin_cycles = 0;
	}
	bool sendresult = false;
	if (checkin_cycles == 0 || checkin_cycles >= checkin_rate / wake_rate) {
		info("Sending GPS event"); 
		sendresult = attempt(&sendToServer);
		if (sendresult) {
			checkin_cycles = 0;
		}
	}
	last_gps_operation = result;
	info("Checkin cycles:");
	Serial.println(checkin_cycles);
	return sendresult;
}

/*****************************************************

Fona Status Checks

******************************************************/
bool setRTCTimeStamp() {
	if (hardware_rtc_present) {
		// get hardware RTC value
		return true;
	}
	return false;
}

bool getTemperature() {
	clearBuffer();
	fona.getTemperatureInfo(buffer, TEMPERATURE_LENGTH);
	if (postError()) return false;
	setPostData(buffer, TEMPERATURE_INDEX);
	printPostData();
	char *token = strtok_P(buffer, GPS_TOKEN);
	token = strtok_P(NULL, GPS_TOKEN);
	token = strtok_P(NULL, GPS_TOKEN);
	if (strlen(token) > 0) {
		info("Critical battery temperature");
		notify_temperature_critical = true;
	}
	return true;
}

bool getBattery() {
	clearBuffer();
	fona.getBattInfo(buffer, BATTERY_LENGTH);
	clearPostData(BATTERY_INDEX, BATTERY_LENGTH);
	setPostData(buffer, BATTERY_INDEX);
	printPostData();
	char* token = strtok_P(buffer, GPS_TOKEN);
	chargeStatus = atoi(token);
	token = strtok_P(NULL, GPS_TOKEN);
	int newBattPercent = atoi(token);
#ifdef SIMULATE
	newBattPercent = 10;
#endif
	if (newBattPercent < battPercent && newBattPercent < 15 && chargeStatus == 0) {
		notify_battery_low = true;
#ifdef SIMULATE
		Serial.println(F("Low battery notification"));
#endif
	}
	battPercent = newBattPercent;
	token = strtok_P(NULL, GPS_TOKEN);
	battMilliVolts = atoi(token);
	return true;
}


int getGPS() {
	clearBuffer();

	char num[16] = { 0 };
	fona.getGPS(32, buffer, BUFFER_LENGTH);

	char * token = strtok_P(buffer, GPS_TOKEN);
	if (token[0] == '0') {
		info("No GPS lock");
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
		info("No GPS lock");
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
		stationaryCount++;
		return GPS_NO_CHANGE;
	}
	lat = newLat;
	lng = newLng;
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	setPostData(latitude, LOCATION_INDEX);
	postdata[LOCATION_INDEX + strlen(latitude)] = ',';
	setPostData(longitude, LOCATION_INDEX + strlen(latitude) + 1);
	clearPostData(HEADING_INDEX, HEADING_LENGTH);
	setPostData(altitude, HEADING_INDEX);
	int nextIndex = HEADING_INDEX + strlen(altitude);
	postdata[nextIndex] = ',';
	nextIndex++;
	setPostData(speed, nextIndex);
	nextIndex += strlen(speed);
	postdata[nextIndex] = ',';
	nextIndex++;
	setPostData(heading, nextIndex);
	clearBuffer();
	sprintf_P(buffer, TIMESTAMP_FORMAT, current_time.unixtime());
	setPostData(buffer, TIMESTAMP_INDEX);
	printPostData();
	stationaryCount = 0;
	return GPS_CHANGED;
}

/*****************************************************************

Setup and Loop

******************************************************************/

void setup() {
	pinMode(FONA_KEY_PIN, OUTPUT);
	pinMode(FONA_POWER_PIN, INPUT);
	pinMode(FONA_DTR_PIN, OUTPUT);
	digitalWrite(FONA_DTR_PIN, LOW);
	digitalWrite(FONA_KEY_PIN, LOW);
	bool force_arm = false;
	pinMode(CHARGE_DETECT_PIN, INPUT_PULLUP);

	if (analogRead(CHARGE_DETECT_PIN) == 1023) {
		mode = MODE_CHARGE;
	}
#ifdef FORCE_CHARGE
	mode = MODE_CHARGE;
#endif

	if (mode == MODE_CHARGE) {
		Serial.begin(BAUD_RATE);
		info("Tracker is charging");
		fonaColdStart();
		getBattery();
		getTemperature();

		fona_sleep();
		while (1) {									// Charging only				
			digitalWrite(FONA_DTR_PIN, LOW);
			delay(60000);
			digitalWrite(FONA_DTR_PIN, HIGH);
			getBattery();
			delay(1000);
		}
	}


	clearAllData();
	while (!Serial);
	Serial.begin(BAUD_RATE);

	while (!fonaInit());									// FONA must init!
	accelerometer_present = accelerometerInit();

	getBattery();
	getTemperature();


	attempt(&logBoot);
	if (accelerometer_present) {
		attempt(&logSleep);
		mode = MODE_WATCHDOG;
	}
	else {
		mode = MODE_TRACKER;
	}
#ifndef SLEEP_DISABLED
	initWDT();
	setSleep();
#endif
}

void loop() {
#ifndef SLEEP_DISABLED
	doSleepTimer();
#endif
	if (mode == MODE_WATCHDOG && accelerometer_present && accelerometer_interrupt) {
		info("Accelerometer interrupt!!!!");
		attempt(&logWake);
		mode = MODE_TRACKER;
		setSleep();
	}
#ifdef SLEEP_DISABLED
	wake_timer_expired = true;
#endif
	if (wake_timer_expired) {
		info("Wake timer expired.");
		getBattery();
		getTemperature();
		logGPS();
		if (notify_battery_low) {
			logBattery();
		}
		if (notify_temperature_critical) {
			logTemperature();
		}
		setSleep();
	}
#ifdef SLEEP_DISABLED
	delay(8000);
#endif
}
