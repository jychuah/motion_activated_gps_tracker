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

#define UID			"d76db2b8-be35-477c-a428-2623d523fbfd"
#define IMEI		"865067020757418"
#define HELPER_URL	"http://webpersistent.com/motorbike-tracker/helper/post.php"


#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define UID_INDEX 11
#define IMEI_INDEX 60
#define EVENT_INDEX 89
#define DATA_INDEX 118

char postdata[] = "{ \"uid\" : \"d76db2b8-be35-477c-a428-2623d523fbfd\", "
	"\"imei\" : \"865067020757418\", \"event\" : \"event_type_string\","
	"\"data\" : \"01234567891123456789212345678931234567894123456789512345678961234567897123456789\"}";

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

	Serial.begin(115200);
	Serial.println(F("FONA basic test"));
	Serial.println(F("Initializing....(May take 3 seconds)"));

	fonaSerial->begin(4800);
	if (!fona.begin(*fonaSerial)) {
		Serial.println(F("Couldn't find FONA"));
		while (1);
	}
	type = fona.type();
	Serial.println(F("FONA is OK"));
	Serial.print(F("Found "));
	switch (type) {
	case FONA800L:
		Serial.println(F("FONA 800L")); break;
	case FONA800H:
		Serial.println(F("FONA 800H")); break;
	case FONA808_V1:
		Serial.println(F("FONA 808 (v1)")); break;
	case FONA808_V2:
		Serial.println(F("FONA 808 (v2)")); break;
	case FONA3G_A:
		Serial.println(F("FONA 3G (American)")); break;
	case FONA3G_E:
		Serial.println(F("FONA 3G (European)")); break;
	default:
		Serial.println(F("???")); break;
	}

	// Print SIM card IMEI number.
	char imei[15] = { 0 }; // MUST use a 16 character buffer for IMEI!
	uint8_t imeiLen = fona.getIMEI(imei);
	if (imeiLen > 0) {
		Serial.print("SIM card IMEI: "); Serial.println(imei);
	}
	strncpy(&postdata[IMEI_INDEX], imei, 15);
	strncpy(&postdata[UID_INDEX], UID, strlen(UID));
	Serial.println(postdata);

	// Optionally configure a GPRS APN, username, and password.
	// You might need to do this to access your network's GPRS/data
	// network.  Contact your provider for the exact APN, username,
	// and password values.  Username and password are optional and
	// can be removed, but APN is required.
	fona.setGPRSNetworkSettings(F("fast.t-mobile.com"));

	// Optionally configure HTTP gets to follow redirects over SSL.
	// Default is not to follow SSL redirects, however if you uncomment
	// the following line then redirects over SSL will be followed.
	//fona.setHTTPSRedirect(true);

	printMenu();
}


void printMenu(void) {
	Serial.println(F("-------------------------------------"));
	Serial.println(F("[?] Print this menu"));
	Serial.println(F("[a] read the ADC 2.8V max (FONA800 & 808)"));
	Serial.println(F("[b] read the Battery V and % charged"));
	Serial.println(F("[C] read the SIM CCID"));
	Serial.println(F("[U] Unlock SIM with PIN code"));
	Serial.println(F("[i] read RSSI"));
	Serial.println(F("[n] get Network status"));
	Serial.println(F("[P] PWM/Buzzer out (FONA800 & 808)"));

	// Time
	Serial.println(F("[y] Enable network time sync (FONA 800 & 808)"));
	Serial.println(F("[Y] Enable NTP time sync (GPRS FONA 800 & 808)"));
	Serial.println(F("[t] Get network time"));

	// GPRS
	Serial.println(F("[G] Enable GPRS"));
	Serial.println(F("[g] Disable GPRS"));
	Serial.println(F("[l] Query GSMLOC (GPRS)"));
	Serial.println(F("[w] Read webpage (GPRS)"));
	Serial.println(F("[W] Post to website (GPRS)"));

	// GPS
	if ((type == FONA3G_A) || (type == FONA3G_E) || (type == FONA808_V1) || (type == FONA808_V2)) {
		Serial.println(F("[O] Turn GPS on (FONA 808 & 3G)"));
		Serial.println(F("[o] Turn GPS off (FONA 808 & 3G)"));
		Serial.println(F("[L] Query GPS location (FONA 808 & 3G)"));
		if (type == FONA808_V1) {
			Serial.println(F("[x] GPS fix status (FONA808 v1 only)"));
		}
		Serial.println(F("[E] Raw NMEA out (FONA808)"));
	}
	Serial.println(F("[F] Firebase test"));

	Serial.println(F("[S] create Serial passthru tunnel"));
	Serial.println(F("-------------------------------------"));
	Serial.println(F(""));

}
void loop() {
	Serial.print(F("FONA> "));
	while (!Serial.available()) {
		if (fona.available()) {
			Serial.write(fona.read());
		}
	}

	char command = Serial.read();
	Serial.println(command);


	switch (command) {
	case '?': {
		printMenu();
		break;
	}

	case 'a': {
		// read the ADC
		uint16_t adc;
		if (!fona.getADCVoltage(&adc)) {
			Serial.println(F("Failed to read ADC"));
		}
		else {
			Serial.print(F("ADC = ")); Serial.print(adc); Serial.println(F(" mV"));
		}
		break;
	}

	case 'b': {
		// read the battery voltage and percentage
		uint16_t vbat;
		if (!fona.getBattVoltage(&vbat)) {
			Serial.println(F("Failed to read Batt"));
		}
		else {
			Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
		}


		if (!fona.getBattPercent(&vbat)) {
			Serial.println(F("Failed to read Batt"));
		}
		else {
			Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
		}

		break;
	}

	case 'U': {
		// Unlock the SIM with a PIN code
		char PIN[5];
		flushSerial();
		Serial.println(F("Enter 4-digit PIN"));
		readline(PIN, 3);
		Serial.println(PIN);
		Serial.print(F("Unlocking SIM card: "));
		if (!fona.unlockSIM(PIN)) {
			Serial.println(F("Failed"));
		}
		else {
			Serial.println(F("OK!"));
		}
		break;
	}

	case 'C': {
		// read the CCID
		fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
		Serial.print(F("SIM CCID = ")); Serial.println(replybuffer);
		break;
	}

	case 'i': {
		// read the RSSI
		uint8_t n = fona.getRSSI();
		int8_t r;

		Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
		if (n == 0) r = -115;
		if (n == 1) r = -111;
		if (n == 31) r = -52;
		if ((n >= 2) && (n <= 30)) {
			r = map(n, 2, 30, -110, -54);
		}
		Serial.print(r); Serial.println(F(" dBm"));

		break;
	}

	case 'n': {
		// read the network/cellular status
		uint8_t n = fona.getNetworkStatus();
		Serial.print(F("Network status "));
		Serial.print(n);
		Serial.print(F(": "));
		if (n == 0) Serial.println(F("Not registered"));
		if (n == 1) Serial.println(F("Registered (home)"));
		if (n == 2) Serial.println(F("Not registered (searching)"));
		if (n == 3) Serial.println(F("Denied"));
		if (n == 4) Serial.println(F("Unknown"));
		if (n == 5) Serial.println(F("Registered roaming"));
		break;
	}

			  /*** PWM ***/

	case 'P': {
		// PWM Buzzer output @ 2KHz max
		flushSerial();
		Serial.print(F("PWM Freq, 0 = Off, (1-2000): "));
		uint16_t freq = readnumber();
		Serial.println();
		if (!fona.setPWM(freq)) {
			Serial.println(F("Failed"));
		}
		else {
			Serial.println(F("OK!"));
		}
		break;
	}


			  /*** Time ***/

	case 'y': {
		// enable network time sync
		if (!fona.enableNetworkTimeSync(true))
			Serial.println(F("Failed to enable"));
		break;
	}

	case 'Y': {
		// enable NTP time sync
		if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
			Serial.println(F("Failed to enable"));
		break;
	}

	case 't': {
		// read the time
		char buffer[23];

		fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
		Serial.print(F("Time = ")); Serial.println(buffer);
		break;
	}


			  /*********************************** GPS (SIM808 only) */

	case 'o': {
		// turn GPS off
		if (!fona.enableGPS(false))
			Serial.println(F("Failed to turn off"));
		break;
	}
	case 'O': {
		// turn GPS on
		if (!fona.enableGPS(true))
			Serial.println(F("Failed to turn on"));
		break;
	}
	case 'x': {
		int8_t stat;
		// check GPS fix
		stat = fona.GPSstatus();
		if (stat < 0)
			Serial.println(F("Failed to query"));
		if (stat == 0) Serial.println(F("GPS off"));
		if (stat == 1) Serial.println(F("No fix"));
		if (stat == 2) Serial.println(F("2D fix"));
		if (stat == 3) Serial.println(F("3D fix"));
		break;
	}

	case 'L': {
		// check for GPS location
		char gpsdata[80];
		fona.getGPS(0, gpsdata, 80);
		if (type == FONA808_V1)
			Serial.println(F("Reply in format: mode,longitude,latitude,altitude,utctime(yyyymmddHHMMSS),ttff,satellites,speed,course"));
		else
			Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
		Serial.println(gpsdata);

		break;
	}

	case 'E': {
		flushSerial();
		if (type == FONA808_V1) {
			Serial.print(F("GPS NMEA output sentences (0 = off, 34 = RMC+GGA, 255 = all)"));
		}
		else {
			Serial.print(F("On (1) or Off (0)? "));
		}
		uint8_t nmeaout = readnumber();

		// turn on NMEA output
		fona.enableGPSNMEA(nmeaout);

		break;
	}

			  /*********************************** GPRS */

	case 'g': {
		// turn GPRS off
		if (!fona.enableGPRS(false))
			Serial.println(F("Failed to turn off"));
		break;
	}
	case 'G': {
		// turn GPRS on
		if (!fona.enableGPRS(true))
			Serial.println(F("Failed to turn on"));
		break;
	}
	case 'l': {
		// check for GSMLOC (requires GPRS)
		uint16_t returncode;

		if (!fona.getGSMLoc(&returncode, replybuffer, 250))
			Serial.println(F("Failed!"));
		if (returncode == 0) {
			Serial.println(replybuffer);
		}
		else {
			Serial.print(F("Fail code #")); Serial.println(returncode);
		}

		break;
	}
	case 'w': {
		// read website URL
		uint16_t statuscode;
		int16_t length;
		char url[80];

		flushSerial();
		Serial.println(F("NOTE: in beta! Use small webpages to read!"));
		Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
		Serial.print(F("http://")); readline(url, 79);
		Serial.println("Echoing URL:");
		Serial.println(url);

		Serial.println(F("****"));
		if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
			Serial.print("Status code:");
			Serial.println(statuscode);
			Serial.print("Length: ");
			Serial.println(length);
			Serial.println("Failed!");
			break;
		}
		while (length > 0) {
			while (fona.available()) {
				char c = fona.read();

				// Serial.write is too slow, we'll write directly to Serial register!
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
				loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
				UDR0 = c;
#else
				Serial.write(c);
#endif
				length--;
				if (!length) break;
			}
		}
		Serial.println(F("\n****"));
		fona.HTTP_GET_end();
		break;
	}

	case 'W': {
		// Post data to website
		uint16_t statuscode;
		int16_t length;
		char url[80];
		char data[80];

		flushSerial();
		Serial.println(F("NOTE: in beta! Use simple websites to post!"));
		Serial.println(F("URL to post (e.g. httpbin.org/post):"));
		Serial.print(F("http://")); readline(url, 79);
		Serial.println(url);
		Serial.println(F("Data to post (e.g. \"foo\" or \"{\"simple\":\"json\"}\"):"));
		readline(data, 79);
		Serial.println(data);

		Serial.println(F("****"));
		if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *)data, strlen(data), &statuscode, (uint16_t *)&length)) {
			Serial.println("Failed!");
			break;
		}
		while (length > 0) {
			while (fona.available()) {
				char c = fona.read();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
				loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
				UDR0 = c;
#else
				Serial.write(c);
#endif

				length--;
				if (!length) break;
			}
		}
		Serial.println(F("\n****"));
		fona.HTTP_POST_end();
		break;
	}

	case 'F': {
		Serial.println("Firebase Test");
		uint16_t statuscode;
		int16_t length;
		String event = "gps";
		String event_data = "coordinate";
		String data_string = "{ \"uid\" : \"" + String(UID) + "\", \"imei\" : \"" + String(IMEI) + "\", \"event\" : \"" + event +
			"\", \"data\" : \"" + event_data + "\"}";
		char data[data_string.length()];
		data_string.toCharArray(data, data_string.length(), 0);
		

		if (!fona.HTTP_POST_start(HELPER_URL, F("text/plain"), (uint8_t *)data, strlen(data), &statuscode, (uint16_t *)&length)) {
			Serial.println("Failed!");
			break;
		}
		Serial.print("length: ");
		Serial.println(length);
		while (length > 0) {
			while (fona.available()) {
				char c = fona.read();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
				loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty. 
				UDR0 = c;
#else
				Serial.write(c);
#endif

				length--;
				if (!length) break;
			}
		}
		Serial.println(F("\n****"));
		fona.HTTP_POST_end();

		
		break;
	}
			  /*****************************************/

	case 'S': {
		Serial.println(F("Creating SERIAL TUBE"));
		while (1) {
			while (Serial.available()) {
				delay(1);
				fona.write(Serial.read());
			}
			if (fona.available()) {
				Serial.write(fona.read());
			}
		}
		break;
	}

	default: {
		Serial.println(F("Unknown command"));
		printMenu();
		break;
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

char readBlocking() {
	while (!Serial.available());
	return Serial.read();
}
uint16_t readnumber() {
	uint16_t x = 0;
	char c;
	while (!isdigit(c = readBlocking())) {
		//Serial.print(c);
	}
	Serial.print(c);
	x = c - '0';
	while (isdigit(c = readBlocking())) {
		Serial.print(c);
		x *= 10;
		x += c - '0';
	}
	return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
	uint16_t buffidx = 0;
	boolean timeoutvalid = true;
	if (timeout == 0) timeoutvalid = false;

	while (true) {
		if (buffidx > maxbuff) {
			//Serial.println(F("SPACE"));
			break;
		}

		while (Serial.available()) {
			char c = Serial.read();

			//Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

			if (c == '\r') continue;
			if (c == 0xA) {
				if (buffidx == 0)   // the first 0x0A is ignored
					continue;

				timeout = 0;         // the second 0x0A is the end of the line
				timeoutvalid = true;
				break;
			}
			buff[buffidx] = c;
			buffidx++;
		}

		if (timeoutvalid && timeout == 0) {
			//Serial.println(F("TIMEOUT"));
			break;
		}
		delay(1);
	}
	buff[buffidx] = 0;  // null term
	return buffidx;
}