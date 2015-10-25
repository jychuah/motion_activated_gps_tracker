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
#define		IMEI				"865067020757418"
/**************************************************************************************/
#define DEBUG true

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_MAX_ATTEMPTS 5
#define FONA_DELAY 5000

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
					"\"sequence\" : \"-K1B0_C2_tb5uNlFeutR\", "
					"\"location\" : \"-180.123456, -180.123456\", "
					"\"timestamp\" : \"1444091176\", "
					"\"type\" : \"event_type_string\","
					"\"data\" : \"0123456789012345678901234567890123456789\" }";

#define BUFFER_LENGTH 120
char buffer[BUFFER_LENGTH];

#define clearBuffer()		memset(buffer, 0, BUFFER_LENGTH)

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
volatile int sleep_cycles = 0;
volatile int f_wdt = 1;

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;
bool accelerometer_interrupt = false;
bool wake_timer_expired = false;
int wake_rate = 60;


void setup() {
	while (!Serial);
	Serial.begin(9600);

	fona_present = fonaInit();
	accelerometer_present = accelerometerInit();
#ifdef SLEEP_ENABLED
	initWDT();
	enterSleep();
#endif
}

void loop() {
#ifdef SLEEP_ENABLED
	doSleepTimer();
#endif
	if (accelerometer_present && accelerometer_interrupt) {
		Serial.println(F("Accelerometer interrupt!!!!"));
	}
	if (wake_timer_expired) {
		Serial.println(F("Wake timer expired."));
	}
	delay(1000);
}

/*******************************************************************

Hardware Init Functions

********************************************************************/

bool fonaInit() {
	Serial.println(F("FONA basic test"));
	Serial.println(F("Initializing....(May take 3 seconds)"));

	fonaSerial->begin(9600);

	if (!fona.begin(*fonaSerial)) {
		Serial.println(F("Couldn't find FONA"));
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

	int attempts;

	attempts = 0;
	Serial.println(F("Enabling GPRS"));
	while (!fona.enableGPRS(true) && attempts < FONA_MAX_ATTEMPTS) delay(FONA_DELAY + attempts * 1000);
	if (attempts >= FONA_MAX_ATTEMPTS) return false;
	
	attempts = 0;
	Serial.println(F("Enabling GPS"));
	while (!fona.enableGPS(true) && attempts < FONA_MAX_ATTEMPTS) delay(FONA_DELAY + attempts * 1000);
	if (attempts >= FONA_MAX_ATTEMPTS) return false;

	Serial.println(F("Fona Init Successful"));
	return true;
}

bool accelerometerInit() {
	if (!mma.begin()) {
		Serial.println(F("Coudln't initialize MMA8451 Accelerometer"));
		return false;
	}
	else {
		Serial.println(F("MMA8451 Accelerometer OK"));
		return true;
	}
}

/*******************************************************************

Sleep Functions

********************************************************************/


void doSleepTimer() {
	if (sleep_cycles < (wake_rate / 8) && !accelerometer_interrupt && !wake_timer_expired) {
		sleep_cycles++;;
		if (mma.motionDetected()) {				// detect motion and clear latch
			accelerometer_interrupt = true;
		}
		else {
			delay(100);
			enterSleep();
		}
	}
	else {
		wake_timer_expired = true;
	}
}

void initWDT() {
	Serial.println(F("Initializing Watchdog Timer"));
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
	Serial.println(F("Entering sleep"));
	delay(1000);
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
	if (hardware_rtc_present) {
		return true;
	}
	else {
		if (fona_present) {
			clearBuffer();
			fona.getTime(buffer, TIMESTAMP_LENGTH + 1);
			strncpy(&postdata[TIMESTAMP_INDEX], buffer, TIMESTAMP_LENGTH);
#ifdef DEBUG
			Serial.println("Timestamp postdata:");
			Serial.println(postdata);
#endif		
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

bool logWake() {
	setEvent(WAKE_EVENT);
#ifdef DEBUG
	Serial.println("Wake postdata");
	Serial.println(postdata);
#endif
#ifndef DEBUG
	if (!sendPostData()) return false;
#endif
	return postError();
}


bool logBoot() {
	int attempts = 0;
	Serial.println(F("Starting sequence"));
	while (!newSequence() && attempts < FONA_MAX_ATTEMPTS) delay(FONA_DELAY + attempts * 1000);
	if (attempts >= FONA_MAX_ATTEMPTS) return false;
}


bool logBattery() {
	uint16_t vbat;
	if (fona.getBattPercent(&vbat)) {
		if (vbat < 10) {
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

bool logGPS() {
	clearBuffer();
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	clearPostData(DATA_INDEX, DATA_LENGTH);

	char num[16] = { 0 };
	fona.getGPS(32, buffer, BUFFER_LENGTH);

	char * token = strtok_P(buffer, GPS_TOKEN);
	if (token[0] == '0') {
		Serial.println(F("No GPS lock"));
		return false;
	}
	token = strtok_P(NULL, GPS_TOKEN);		// check lock token
	if (token[0] == '0') {
		Serial.println(F("No GPS lock"));
		return false;
	}

	token = strtok_P(NULL, GPS_TOKEN);		// we are now at date
	token = strtok_P(NULL, GPS_TOKEN);		// latitude
	setPostData(token, LOCATION_INDEX);
	postdata[LOCATION_INDEX + strlen(token)] = ',';
	int dataIndex = LOCATION_INDEX + strlen(token) + 1;

	token = strtok_P(NULL, GPS_TOKEN);		// longitude
	setPostData(token, dataIndex);

	token = strtok_P(NULL, GPS_TOKEN);		// altitude
	setPostData(token, DATA_INDEX);
	dataIndex = DATA_INDEX + strlen(token);
	postdata[dataIndex] = ',';
	dataIndex++;

	token = strtok_P(NULL, GPS_TOKEN);		// speed (?)
	setPostData(token, dataIndex);
	dataIndex += strlen(token);
	postdata[dataIndex] = ',';
	dataIndex++;

	token = strtok_P(NULL, GPS_TOKEN);		// heading (?)
	setPostData(token, dataIndex);


	setEvent(GPS);
#ifdef DEBUG
	Serial.println("GPS postdata: ");
	Serial.println(postdata);
#endif
#ifndef DEBUG
	if (!sendPostData()) return false;
#endif
	if (postError()) {
		return false;
	}
	return true;
}

bool newSequence() {
	clearBuffer();
#ifdef DEBUG
	Serial.println("Boot postdata:");
#endif
#ifndef DEBUG
	if (!sendPostData()) return false;
#endif
	if (postError()) {
		return false;
	}
#ifdef DEBUG
	Serial.println("Boot response:");
	Serial.println(postdata);
#endif
	setEvent(BOOT);
	strncpy(&postdata[SEQUENCE_INDEX], buffer, strlen(buffer));
#ifdef DEBUG
	Serial.println("Boot postdata after response:");
	Serial.println(postdata);
#endif
	return strlen(buffer) == SEQUENCE_LENGTH;
}

void flushSerial() {
	while (Serial.available())
		Serial.read();
}

