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

// Format Strings
const char LOCATION_FORMAT[] PROGMEM = "%.6f, %.6f";
const char GPS_DATA_FORMAT[] PROGMEM = "%.2f, %.2f, %.2f";
const char BATTERY_DATA_FORMAT[] PROGMEM = "%i";
const char TEMPERATURE_DATA_FORMAT[] PROGMEM = "%.2f";

//JSON post data indexes and lengths
#define UID_INDEX 11
#define IMEI_INDEX 60
#define SEQUENCE_INDEX 92
#define LOCATION_INDEX 129
#define TYPE_INDEX 166
#define DATA_INDEX 195

#define UID_LENGTH 36
#define IMEI_LENGTH 15
#define SEQUENCE_LENGTH 20
#define LOCATION_LENGTH 24
#define TYPE_LENGTH 17
#define DATA_LENGTH 40

//JSON post buffer
char postdata[] = "{ \"uid\" : \"axxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxz\", "
					"\"imei\" : \"012345678901234\", "
					"\"sequence\" : \"-K1B0_C2_tb5uNlFeutR\", "
					"\"location\" : \"-180.123456, -180.123456\", "
					"\"type\" : \"event_type_string\","
					"\"data\" : \"01234567890123456789012345678901234567890123456789\" }";

#define POST_RESULT_LENGTH 30
char postresult[POST_RESULT_LENGTH];

// Motorbike Tracker Hardware Pins
#define ACCEL_INT2_PIN 2


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

bool motion_detector = false;
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

	fonaInit();
	accelerometerInit();
	initWDT();
	enterSleep();
}

void loop() {
	doSleepTimer();
	if (accelerometer_interrupt) {
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
	char imei[15] = { 0 }; // MUST use a 16 character buffer for IMEI!
	uint8_t imeiLen = fona.getIMEI(imei);

	// Set IMEI in post data
	strncpy(&postdata[IMEI_INDEX], imei, 15);

	// Set UID in post data
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	fona.setGPRSNetworkSettings(F(APN));

	Serial.println(F("Enabling GPRS"));
	while (!fona.enableGPRS(true)) delay(5000);

	Serial.println(F("Enabling GPS"));
	while (!fona.enableGPS(true)) delay(5000);

	Serial.println(F("Starting sequence"));
	while (!newSequence()) delay(5000);
}

void accelerometerInit() {
	if (!mma.begin()) {
		Serial.println(F("Coudln't initialize MMA8451 Accelerometer"));
	}
	else {
		Serial.println(F("MMA8451 Accelerometer OK"));
	}
	pinMode(2, INPUT);
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
void accelerometerISR() {
	sleep_disable();
	detachInterrupt(digitalPinToInterrupt(ACCEL_INT2_PIN));
	accelerometer_interrupt = true;
	Serial.println(F("Accelerometer Interrupt"));
}


/*******************************************************************

Post Functions

********************************************************************/
bool postError(void) {
	return strncmp_P(postresult, ERROR, strlen(ERROR)) == 0;
}

void setPostData(char *data, int index) {
	strncpy(&postdata[index], data, strlen(data));
}

void clearPostData(int index, int length) {
	memset(&postdata[index], ' ', length);
}

void setEvent(const char *event) {
	clearPostData(TYPE_INDEX, TYPE_LENGTH); 
	strncpy_P(&postdata[TYPE_INDEX], event, strlen(event));
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


/*******************************************************************

Tracker Event Functions

********************************************************************/

bool logWakeEvent() {
	setEvent(WAKE_EVENT);
	if (!sendPostData()) return false;
	return postError();
}

bool checkLowBattery() {
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

bool logGPSLocation() {
	float lat = 0;
	float lng = 0;
	float speed_kph = 0;
	float heading = 0;
	float altitude = 0;
	if (!fona.getGPS(&lat, &lng, &speed_kph, &heading, &altitude)) {
		return false;
	}
	clearPostData(LOCATION_INDEX, LOCATION_LENGTH);
	clearPostData(DATA_INDEX, DATA_LENGTH);
	setEvent(GPS);
	sprintf_P(&postdata[LOCATION_INDEX], LOCATION_FORMAT, lat, lng);
	sprintf_P(&postdata[DATA_INDEX], GPS_DATA_FORMAT, speed_kph, heading, altitude);
	if (!sendPostData()) return false;
	if (postError()) {
		return false;
	}
	return true;
}

bool newSequence() {
	setEvent(BOOT);
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

