/***************************************************
This is an example for our Adafruit FONA Cellular Module

Designed specifically to work with the Adafruit FONA
----> http://www.adafruit.com/products/1946
----> http://www.adafruit.com/products/1963
----> http://www.adafruit.com/products/2468
----> http://www.adafruit.com/products/2542

These cellular modules use TTL Serial to communicate, 2 pins are
required to interface
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, all text above must be included in any redistribution
****************************************************/

/*
THIS CODE IS STILL IN PROGRESS!

Open up the serial console on the Arduino at 115200 baud to interact with FONA

Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
*/
#include "Adafruit_FONA.h"

#define APN			"fast.t-mobile.com"
#define UID			"d76db2b8-be35-477c-a428-2623d523fbfd"
#define IMEI		"865067020757418"
#define HELPER_URL	"http://webpersistent.com/motorbike-tracker/helper/post.php"

#define DEBUG true

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define UID_INDEX 11
#define IMEI_INDEX 60
#define EVENT_INDEX 89
#define DATA_INDEX 118
// Sequence IDs are 20 characters, +4 padding characters
#define SEQUENCE_INDEX 216

#define setEvent(a)	strncpy(&postdata[EVENT_INDEX], a, strlen(a));

// Event strings
const char BATTERY[] PROGMEM = "Battery";
const char NEW_SEQUENCE[] PROGMEM = "NEW SEQUENCE";
const char GPS_LOCATION[] PROGMEM = "GPS Location";

char postdata[] = "{ \"uid\" : \"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\", "
	"\"imei\" : \"xxxxxxxxxxxxxxx\", \"event\" : \"event_type_string\","
	"\"data\" : \"01234567891123456789212345678931234567894123456789512345678961234567897123456789"
	"8123456789912345678901234567891123456789\", \"sequence\" : \"xxxxxxxxxxxxxxxxxxxx    \" }";
char postresult[30];

// this is a large buffer for replies
char replybuffer[255];

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
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

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

	initPostData();

#ifdef DEBUG
	Serial.print("Post data: ");
	Serial.println(postdata);
#endif
	// Optionally configure a GPRS APN, username, and password.
	// You might need to do this to access your network's GPRS/data
	// network.  Contact your provider for the exact APN, username,
	// and password values.  Username and password are optional and
	// can be removed, but APN is required.
	fona.setGPRSNetworkSettings(F(APN));

	// Optionally configure HTTP gets to follow redirects over SSL.
	// Default is not to follow SSL redirects, however if you uncomment
	// the following line then redirects over SSL will be followed.
	//fona.setHTTPSRedirect(true);

#ifdef DEBUG
	Serial.println(F("Enabling GPRS"));
#endif
	while(!fona.enableGPRS(true));

#ifdef DEBUG
	Serial.println(F("Enabling GPS"));
#endif
	while(!fona.enableGPS(true));

// Test sequence
#ifdef DEBUG
	Serial.println(F("Starting sequence"));
	while (!newSequence());
#endif
}

void initPostData() {
	memset(&postdata[EVENT_INDEX], ' ', 17);
	memset(&postdata[DATA_INDEX], ' ', 120);
	memset(&postdata[SEQUENCE_INDEX], ' ', 24);
}

bool sendPostData() {
	uint16_t statuscode;
	int16_t length;

	if (!fona.HTTP_POST_start(HELPER_URL, F("text/plain"), (uint8_t *)postdata, strlen(postdata), &statuscode, (uint16_t *)&length)) {
#ifdef DEBUG
		Serial.println("Failed!");
#endif
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
			if (!length || i >= 30) break;
		}
	}
	memset(&postresult[i], 0, 30 - i);
	Serial.println(F("\n"));
	fona.HTTP_POST_end();

#ifdef DEBUG
	Serial.print(F("Post result: "));
	Serial.println(postresult);
#endif

	return true;
}

void checkBattery() {
	uint16_t vbat;

	if (fona.getBattPercent(&vbat)) {
		if (vbat < 10) {
			initPostData();
			setEvent(BATTERY);
			// TODO: copy battery to data...

			sendPostData();
		} 
	}
#ifdef DEBUG
	else {
		Serial.print(F("Failed to read battery"));
	}
#endif	
}

void logGPSLocation() {
	initPostData();
	setEvent(GPS_LOCATION);
	fona.getGPS(0, &postdata[DATA_INDEX], 120);
	sendPostData();
#ifdef DEBUG
	Serial.print(F("GPS Data: "));
	Serial.println(postdata[DATA_INDEX], 120);
#endif
}

bool newSequence() {
	//TODO: Test
	initPostData();
	setEvent(NEW_SEQUENCE);
	sendPostData();
	if (strlen(postresult) == 20) {
		strncpy(&postdata[SEQUENCE_INDEX], postresult, strlen(postresult));
		return true;
	}
	else {
		return false;
	}
}

void loop() {
	Serial.print(F("FONA> "));
	while (!Serial.available()) {
		if (fona.available()) {
			Serial.write(fona.read());
		}
	}
	// flush input
	flushSerial();
	while (fona.available()) {
		Serial.write(fona.read());
	}

}

void flushSerial() {
	while (Serial.available())
		Serial.read();
}
