/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include <avr/pgmspace.h>
    // next line per http://postwarrior.com/arduino-ethershield-error-prog_char-does-not-name-a-type/
#define prog_char  char PROGMEM

#if (ARDUINO >= 100)
  #include "Arduino.h"
  #ifndef __SAM3X8E__  // Arduino Due doesn't support SoftwareSerial
    #include <SoftwareSerial.h>
  #endif
#else
  #include "WProgram.h"
  #include <NewSoftSerial.h>
#endif

#include "Adafruit_FONA.h"

Adafruit_FONA::Adafruit_FONA(int8_t rst)
{
  _rstpin = rst;

  apn = F("FONAnet");
  apnusername = 0;
  apnpassword = 0;
  mySerial = 0;
  httpsredirect = false;
  useragent = F("FONA");
}

uint8_t Adafruit_FONA::type(void) {
  return _type;
}

boolean Adafruit_FONA::begin(Stream &port) {
  mySerial = &port;

  pinMode(_rstpin, OUTPUT);
  digitalWrite(_rstpin, HIGH);
  delay(10);
  digitalWrite(_rstpin, LOW);
  delay(100);
  digitalWrite(_rstpin, HIGH);

  // give 7 seconds to reboot
  delay(7000);

  while (mySerial->available()) mySerial->read();

  sendCheckReply(F("AT"), F("OK"));
  delay(100);
  sendCheckReply(F("AT"), F("OK"));
  delay(100);
  sendCheckReply(F("AT"), F("OK"));
  delay(100);

  // turn off Echo!
  sendCheckReply(F("ATE0"), F("OK"));
  delay(100);

  if (! sendCheckReply(F("ATE0"), F("OK"))) {
    return false;
  }

  // turn on hangupitude
  sendCheckReply(F("AT+CVHU=0"), F("OK"));

  delay(100);
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print("\t---> "); Serial.println("ATI");
#endif

  mySerial->println("ATI");

  readline(500, true);

#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif

  if (strstr_P(replybuffer, (prog_char *)F("SIM808 R14")) != 0) {
    _type = FONA808_V2;
  } else if (strstr_P(replybuffer, (prog_char *)F("SIM808 R13")) != 0) {
    _type = FONA808_V1;
  } else if (strstr_P(replybuffer, (prog_char *)F("SIM800 R13")) != 0) {
    _type = FONA800L;
  } else if (strstr_P(replybuffer, (prog_char *)F("SIMCOM_SIM5320A")) != 0) {
    _type = FONA3G_A;
  } else if (strstr_P(replybuffer, (prog_char *)F("SIMCOM_SIM5320E")) != 0) {
    _type = FONA3G_E;
  }

  return true;
}


/********* Serial port ********************************************/
boolean Adafruit_FONA::setBaudrate(uint16_t baud) {
  return sendCheckReply(F("AT+IPREX="), baud, F("OK"));
}

/******* Power Down ********/
boolean Adafruit_FONA::powerDown() {
	uint16_t mode = 0;
	return sendCheckReply(F("AT+CPOWD="), mode, F("OK"));
}

boolean Adafruit_FONA::enableSleep(bool status) {
	return sendCheckReply(F("AT+CSCLK="), status ? 1 : 0, F("OK"));
}

/******* Temperature *******/
boolean Adafruit_FONA::enableTemperatureDetection() {
	return sendCheckReply(F("AT+CMTE="), 1, F("OK"));
}

int Adafruit_FONA::getTemperatureInfo(char *buffer, int maxbuff) {
	getReply("AT+CMTE?");
	char *p = strstr_P(replybuffer, (prog_char*)F("+CMTE"));
	if (p == 0) {
		buffer[0] = 0;
		return 0;
	}

	p += 7;

	uint8_t len = min(maxbuff - 1, strlen(p));
	strncpy(buffer, p, len);
	buffer[len] = 0;

	readline(); // eat 'OK'
	return len;
}

/********* Real Time Clock ********************************************/

boolean Adafruit_FONA::readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec) {
  uint16_t v;
  sendParseReply(F("AT+CCLK?"), F("+CCLK: "), &v, '/', 0);
  *year = v;

  Serial.println(*year);
}

boolean Adafruit_FONA::enableRTC(uint8_t i) {
  if (! sendCheckReply(F("AT+CLTS="), i, F("OK")))
    return false;
  return sendCheckReply(F("AT&W"), F("OK"));
}


/********* BATTERY & ADC ********************************************/

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA::getBattVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), v, ',', 2);
}

int Adafruit_FONA::getBattInfo(char *buffer, int maxbuff) {
	getReply("AT+CBC");
	char *p = strstr_P(replybuffer, (prog_char*)F("+CBC"));
	if (p == 0) {
		buffer[0] = 0;
		return 0;
	}

	p += 6;

	uint8_t len = min(maxbuff - 1, strlen(p));
	strncpy(buffer, p, len);
	buffer[len] = 0;

	readline(); // eat 'OK'
	return len;
}

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA_3G::getBattVoltage(uint16_t *v) {
  float f;
  boolean b = sendParseReply(F("AT+CBC"), F("+CBC: "), &f, ',', 2);
  *v = f*1000;
  return b;
}


/* returns the percentage charge of battery as reported by sim800 */
boolean Adafruit_FONA::getBattPercent(uint16_t *p) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), p, ',', 1);
}

boolean Adafruit_FONA::getBattChargeStatus(uint16_t *s) {
	return sendParseReply(F("AT+CBC"), F("+CBC: "), s, ',', 0);
}


boolean Adafruit_FONA::getADCVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CADC?"), F("+CADC: 1,"), v);
}

/********* SIM ***********************************************************/

uint8_t Adafruit_FONA::unlockSIM(char *pin)
{
  char sendbuff[14] = "AT+CPIN=";
  sendbuff[8] = pin[0];
  sendbuff[9] = pin[1];
  sendbuff[10] = pin[2];
  sendbuff[11] = pin[3];
  sendbuff[12] = NULL;

  return sendCheckReply(sendbuff, "OK");
}

uint8_t Adafruit_FONA::getSIMCCID(char *ccid) {
  getReply("AT+CCID");
  // up to 28 chars for reply, 20 char total ccid
  if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(ccid, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(ccid, replybuffer, 20);
  }
  ccid[20] = 0;

  readline(); // eat 'OK'

  return strlen(ccid);
}

/********* IMEI **********************************************************/

uint8_t Adafruit_FONA::getIMEI(char *imei) {
  getReply("AT+GSN");

  // up to 15 chars
  strncpy(imei, replybuffer, 15);
  imei[15] = 0;

  readline(); // eat 'OK'

  return strlen(imei);
}

/********* NETWORK *******************************************************/

uint8_t Adafruit_FONA::getNetworkStatus(void) {
  uint16_t status;

  if (! sendParseReply(F("AT+CREG?"), F("+CREG: "), &status, ',', 1)) return 0;

  return status;
}


uint8_t Adafruit_FONA::getRSSI(void) {
  uint16_t reply;

  if (! sendParseReply(F("AT+CSQ"), F("+CSQ: "), &reply) ) return 0;

  return reply;
}

/********* TIME **********************************************************/

boolean Adafruit_FONA::enableNetworkTimeSync(boolean onoff) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CLTS=1"), F("OK")))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CLTS=0"), F("OK")))
      return false;
  }

  flushInput(); // eat any 'Unsolicted Result Code'

  return true;
}

boolean Adafruit_FONA::enableNTPTimeSync(boolean onoff, const __FlashStringHelper *ntpserver) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CNTPCID=1"), F("OK")))
      return false;

    mySerial->print(F("AT+CNTP=\""));
    if (ntpserver != 0) {
      mySerial->print(ntpserver);
    } else {
      mySerial->print(F("pool.ntp.org"));
    }
    mySerial->println(F("\",0"));
    readline(FONA_DEFAULT_TIMEOUT_MS);
    if (strcmp(replybuffer, "OK") != 0)
      return false;

    if (! sendCheckReply(F("AT+CNTP"), F("OK"), 10000))
      return false;

    uint16_t status;
    readline(10000);
    if (! parseReply(F("+CNTP:"), &status))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CNTPCID=0"), F("OK")))
      return false;
  }

  return true;
}

boolean Adafruit_FONA::getTime(char *buff, uint16_t maxlen) {
  getReply(F("AT+CCLK?"), (uint16_t) 10000);
  if (strncmp(replybuffer, "+CCLK: ", 7) != 0)
    return false;

  char *p = replybuffer+7;
  uint16_t lentocopy = min(maxlen-1, strlen(p));
  strncpy(buff, p, lentocopy+1);
  buff[lentocopy] = 0;

  readline(); // eat OK

  return true;
}

/********* GPS **********************************************************/


boolean Adafruit_FONA::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off

  if (_type == FONA808_V2) {
    if (! sendParseReply(F("AT+CGNSPWR?"), F("+CGNSPWR: "), &state) )
      return false;
  } else {
    if (! sendParseReply(F("AT+CGPSPWR?"), F("+CGPSPWR: "), &state))
      return false;
  }

  if (onoff && !state) {
    if (_type == FONA808_V2) {
      if (! sendCheckReply(F("AT+CGNSPWR=1"), F("OK")))  // try GNS command
	return false;
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=1"), F("OK")))
	return false;
    }
  } else if (!onoff && state) {
    if (_type == FONA808_V2) {
      if (! sendCheckReply(F("AT+CGNSPWR=0"), F("OK"))) // try GNS command
	return false;
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=0"), F("OK")))
	return false;
    }
  }
  return true;
}



boolean Adafruit_FONA_3G::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off
  if (! Adafruit_FONA::sendParseReply(F("AT+CGPS?"), F("+CGPS: "), &state) )
    return false;

  if (onoff && !state) {
    if (! sendCheckReply(F("AT+CGPS=1"), F("OK")))
      return false;
  } else if (!onoff && state) {
    if (! sendCheckReply(F("AT+CGPS=0"), F("OK")))
      return false;
    // this takes a little time
    readline(2000); // eat '+CGPS: 0'
  }
  return true;
}

int8_t Adafruit_FONA::GPSstatus(void) {
  if (_type == FONA808_V2) {
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGNSINF"));
    char *p = strstr_P(replybuffer, (prog_char*)F("+CGNSINF: "));
    if (p == 0) return -1;
    p+=12; // Skip to second value, fix status.
    readline(); // eat 'OK'
    //Serial.println(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 0;
  }
  if (_type == FONA3G_A || _type == FONA3G_E) {
    // FONA 3G doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGPSINFO"));
    char *p = strstr_P(replybuffer, (prog_char*)F("+CGPSINFO:"));
    if (p == 0) return -1;
    if (p[10] != ',') return 3; // if you get anything, its 3D fix
    return 0;
  }
  else {
    // 808 V1 looks for specific 2D or 3D fix state.
    getReply(F("AT+CGPSSTATUS?"));
    char *p = strstr_P(replybuffer, (prog_char*)F("SSTATUS: Location "));
    if (p == 0) return -1;
    p+=18;
    readline(); // eat 'OK'
    //Serial.println(p);
    if (p[0] == 'U') return 0;
    if (p[0] == 'N') return 1;
    if (p[0] == '2') return 2;
    if (p[0] == '3') return 3;
  }
  // else
  return 0;
}

uint8_t Adafruit_FONA::getGPS(uint8_t arg, char *buffer, uint8_t maxbuff) {
  int32_t x = arg;

  if ( (_type == FONA3G_A) || (_type == FONA3G_E) ) {
    getReply(F("AT+CGPSINFO"));
  } else if (_type == FONA808_V1) {
    getReply(F("AT+CGPSINF="), x);
  } else {
    getReply(F("AT+CGNSINF"));
  }

  char *p = strstr_P(replybuffer, (prog_char*)F("SINF"));
  if (p == 0) {
    buffer[0] = 0;
    return 0;
  }

  p+=6;

  uint8_t len = min(maxbuff-1, strlen(p));
  strncpy(buffer, p, len);
  buffer[len] = 0;

  readline(); // eat 'OK'
  return len;
}

boolean Adafruit_FONA::getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude) {

  char gpsbuffer[120];

  // we need at least a 2D fix
  if (GPSstatus() < 2)
    return false;

  // grab the mode 2^5 gps csv from the sim808
  uint8_t res_len = getGPS(32, gpsbuffer, 120);

  // make sure we have a response
  if (res_len == 0)
    return false;

  if (_type == FONA3G_A || _type == FONA3G_E) {
    // Parse 3G respose
    // +CGPSINFO:4043.000000,N,07400.000000,W,151015,203802.1,-12.0,0.0,0
    // skip beginning
    char *tok;

   // grab the latitude
    char *latp = strtok(gpsbuffer, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    // skip date & time
    tok = strtok(NULL, ",");
    tok = strtok(NULL, ",");

   // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;
      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

  } else if (_type == FONA808_V2) {
    // Parse 808 V2 response.  See table 2-3 from here for format:
    // http://www.adafruit.com/datasheets/SIM800%20Series_GNSS_Application%20Note%20V1.00.pdf

    // skip GPS run status
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip fix status
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    *lat = atof(latp);
    *lon = atof(longp);

    // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;

      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }
  }
  else {
    // Parse 808 V1 response.

    // skip mode
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip fix
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

    // only grab speed if needed
    if (speed_kph != NULL) {

      // grab the speed in knots
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      // convert to kph
      *speed_kph = atof(speedp) * 1.852;

    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);

    }

    // no need to continue
    if (altitude == NULL)
      return true;

    // we need at least a 3D fix for altitude
    if (GPSstatus() < 3)
      return false;

    // grab the mode 0 gps csv from the sim808
    res_len = getGPS(0, gpsbuffer, 120);

    // make sure we have a response
    if (res_len == 0)
      return false;

    // skip mode
    tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip lat
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip long
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab altitude
    char *altp = strtok(NULL, ",");
    if (! altp) return false;

    *altitude = atof(altp);
  }

  return true;

}

boolean Adafruit_FONA::enableGPSNMEA(uint8_t i) {

  char sendbuff[15] = "AT+CGPSOUT=000";
  sendbuff[11] = (i / 100) + '0';
  i %= 100;
  sendbuff[12] = (i / 10) + '0';
  i %= 10;
  sendbuff[13] = i + '0';

  if (_type == FONA808_V2) {
    if (i)
      return sendCheckReply(F("AT+CGNSTST=1"), F("OK"));
    else
      return sendCheckReply(F("AT+CGNSTST=0"), F("OK"));
  } else {
    return sendCheckReply(sendbuff, "OK", 2000);
  }
}


/********* GPRS **********************************************************/


boolean Adafruit_FONA::enableGPRS(boolean onoff) {

  if (onoff) {
    // disconnect all sockets
    sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000);

    if (! sendCheckReply(F("AT+CGATT=1"), F("OK"), 10000))
      return false;

    // set bearer profile! connection type GPRS
    if (! sendCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""),
			   F("OK"), 10000))
      return false;

    // set bearer profile access point name
    if (apn) {
      // Send command AT+SAPBR=3,1,"APN","<apn value>" where <apn value> is the configured APN value.
      if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"APN\","), apn, F("OK"), 10000))
        return false;

      // set username/password
      if (apnusername) {
        // Send command AT+SAPBR=3,1,"USER","<user>" where <user> is the configured APN username.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"USER\","), apnusername, F("OK"), 10000))
          return false;
      }
      if (apnpassword) {
        // Send command AT+SAPBR=3,1,"PWD","<password>" where <password> is the configured APN password.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"PWD\","), apnpassword, F("OK"), 10000))
          return false;
      }
    }

    // open GPRS context
    if (! sendCheckReply(F("AT+SAPBR=1,1"), F("OK"), 10000))
      return false;
  } else {
    // disconnect all sockets
    if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000))
      return false;

    // close GPRS context
    if (! sendCheckReply(F("AT+SAPBR=0,1"), F("OK"), 10000))
      return false;

    if (! sendCheckReply(F("AT+CGATT=0"), F("OK"), 10000))
      return false;

  }
  return true;
}

boolean Adafruit_FONA_3G::enableGPRS(boolean onoff) {

  if (onoff) {
    // disconnect all sockets
    //sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000);

    if (! sendCheckReply(F("AT+CGATT=1"), F("OK"), 10000))
      return false;


    // set bearer profile access point name
    if (apn) {
      // Send command AT+CGSOCKCONT=1,"IP","<apn value>" where <apn value> is the configured APN name.
      if (! sendCheckReplyQuoted(F("AT+CGSOCKCONT=1,\"IP\","), apn, F("OK"), 10000))
        return false;

      // set username/password
      if (apnusername) {
	char authstring[100] = "AT+CGAUTH=1,1,\"";
	char *strp = authstring + strlen(authstring);
	strcpy_P(strp, (prog_char *)apnusername);
	strp+=strlen_P((prog_char *)apnusername);
	strp[0] = '\"';
	strp++;
	strp[0] = 0;

	if (apnpassword) {
	  strp[0] = ','; strp++;
	  strp[0] = '\"'; strp++;
	  strcpy_P(strp, (prog_char *)apnpassword);
	  strp+=strlen_P((prog_char *)apnpassword);
	  strp[0] = '\"';
	  strp++;
	  strp[0] = 0;
	}

	if (! sendCheckReply(authstring, "OK", 10000))
	  return false;
      }
    }

    // connect in transparent
    if (! sendCheckReply(F("AT+CIPMODE=1"), F("OK"), 10000))
      return false;
    // open network (?)
    if (! sendCheckReply(F("AT+NETOPEN=,,1"), F("Network opened"), 10000))
      return false;

    readline(); // eat 'OK'
  } else {
    // close GPRS context
    if (! sendCheckReply(F("AT+NETCLOSE"), F("Network closed"), 10000))
      return false;

    readline(); // eat 'OK'
  }

  return true;
}

uint8_t Adafruit_FONA::GPRSstate(void) {
  uint16_t state;

  if (! sendParseReply(F("AT+CGATT?"), F("+CGATT: "), &state) )
    return -1;

  return state;
}

void Adafruit_FONA::setGPRSNetworkSettings(const __FlashStringHelper *apn,
              const __FlashStringHelper *username, const __FlashStringHelper *password) {
  this->apn = apn;
  this->apnusername = username;
  this->apnpassword = password;
}

boolean Adafruit_FONA::getGSMLoc(uint16_t *errorcode, char *buff, uint16_t maxlen) {

  getReply(F("AT+CIPGSMLOC=1,1"), (uint16_t)10000);

  if (! parseReply(F("+CIPGSMLOC: "), errorcode))
    return false;

  char *p = replybuffer+14;
  uint16_t lentocopy = min(maxlen-1, strlen(p));
  strncpy(buff, p, lentocopy+1);

  readline(); // eat OK

  return true;
}

boolean Adafruit_FONA::getGSMLoc(float *lat, float *lon) {

  uint16_t returncode;
  char gpsbuffer[120];

  // make sure we could get a response
  if (! getGSMLoc(&returncode, gpsbuffer, 120))
    return false;

  // make sure we have a valid return code
  if (returncode != 0)
    return false;

  // +CIPGSMLOC: 0,-74.007729,40.730160,2015/10/15,19:24:55
  // tokenize the gps buffer to locate the lat & long
  char *longp = strtok(gpsbuffer, ",");
  if (! longp) return false;

  char *latp = strtok(NULL, ",");
  if (! latp) return false;

  *lat = atof(latp);
  *lon = atof(longp);

  return true;

}
/********* TCP FUNCTIONS  ************************************/


boolean Adafruit_FONA::TCPconnect(char *server, uint16_t port) {
  flushInput();

  // close all old connections
  if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000) ) return false;

  // single connection at a time
  if (! sendCheckReply(F("AT+CIPMUX=0"), F("OK")) ) return false;

  // manually read data
  if (! sendCheckReply(F("AT+CIPRXGET=1"), F("OK")) ) return false;

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print(F("AT+CIPSTART=\"TCP\",\""));
  Serial.print(server);
  Serial.print(F("\",\""));
  Serial.print(port);
  Serial.println(F("\""));
#endif

  mySerial->print(F("AT+CIPSTART=\"TCP\",\""));
  mySerial->print(server);
  mySerial->print(F("\",\""));
  mySerial->print(port);
  mySerial->println(F("\""));

  if (! expectReply(F("OK"))) return false;
  if (! expectReply(F("CONNECT OK"))) return false;
}

boolean Adafruit_FONA::TCPclose(void) {
  return sendCheckReply(F("AT+CIPCLOSE"), F("OK"));
}

boolean Adafruit_FONA::TCPconnected(void) {
  if (! sendCheckReply(F("AT+CIPSTATUS"), F("OK"), 100) ) return false;
  readline(100);
#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return (strcmp(replybuffer, "STATE: CONNECT OK") == 0);
}

boolean Adafruit_FONA::TCPsend(char *packet, uint8_t len) {

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print(F("AT+CIPSEND="));
  Serial.println(len);

  for (uint16_t i=0; i<len; i++) {
    Serial.print(" 0x");
    Serial.print(packet[i], HEX);
  }
  Serial.println();
#endif


  mySerial->print(F("AT+CIPSEND="));
  mySerial->println(len);
  readline();
#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  if (replybuffer[0] != '>') return false;

  mySerial->write(packet, len);
  readline(3000); // wait up to 3 seconds to send the data
#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif

  return (strcmp(replybuffer, "SEND OK") == 0);
}

uint16_t Adafruit_FONA::TCPavailable(void) {
  uint16_t avail;

  if (! sendParseReply(F("AT+CIPRXGET=4"), F("+CIPRXGET: 4,"), &avail, ',', 0) ) return false;

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print (avail); Serial.println(F(" bytes available"));
#endif

  return avail;
}


uint16_t Adafruit_FONA::TCPread(uint8_t *buff, uint8_t len) {
  uint16_t avail;

  mySerial->print(F("AT+CIPRXGET=2,"));
  mySerial->println(len);
  readline();
  if (! parseReply(F("+CIPRXGET: 2,"), &avail, ',', 0)) return false;

  readRaw(avail);

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print (avail); Serial.println(F(" bytes read"));
  for (uint8_t i=0;i<avail;i++) {
    Serial.print(" 0x"); Serial.print(replybuffer[i], HEX);
  }
  Serial.println();
#endif

  memcpy(buff, replybuffer, avail);

  return avail;
}



/********* HTTP LOW LEVEL FUNCTIONS  ************************************/

boolean Adafruit_FONA::HTTP_init() {
  return sendCheckReply(F("AT+HTTPINIT"), F("OK"));
}

boolean Adafruit_FONA::HTTP_term() {
  return sendCheckReply(F("AT+HTTPTERM"), F("OK"));
}

void Adafruit_FONA::HTTP_para_start(const __FlashStringHelper *parameter,
                                    boolean quoted) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> ");
  Serial.print(F("AT+HTTPPARA=\""));
  Serial.print(parameter);
  Serial.println('"');
#endif

  mySerial->print(F("AT+HTTPPARA=\""));
  mySerial->print(parameter);
  if (quoted)
    mySerial->print(F("\",\""));
  else
    mySerial->print(F("\","));
}

boolean Adafruit_FONA::HTTP_para_end(boolean quoted) {
  if (quoted)
    mySerial->println('"');
  else
    mySerial->println();

  return expectReply(F("OK"));
}

boolean Adafruit_FONA::HTTP_para(const __FlashStringHelper *parameter,
                                 const char *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(const __FlashStringHelper *parameter,
                                 const __FlashStringHelper *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(const __FlashStringHelper *parameter,
                                 int32_t value) {
  HTTP_para_start(parameter, false);
  mySerial->print(value);
  return HTTP_para_end(false);
}

boolean Adafruit_FONA::HTTP_data(uint32_t size, uint32_t maxTime) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> ");
  Serial.print(F("AT+HTTPDATA="));
  Serial.print(size);
  Serial.print(",");
  Serial.println(maxTime);
#endif

  mySerial->print(F("AT+HTTPDATA="));
  mySerial->print(size);
  mySerial->print(",");
  mySerial->println(maxTime);

  return expectReply(F("DOWNLOAD"));
}

boolean Adafruit_FONA::HTTP_action(uint8_t method, uint16_t *status,
                                   uint16_t *datalen, int32_t timeout) {
  // Send request.
  if (! sendCheckReply(F("AT+HTTPACTION="), method, F("OK")))
    return false;

  // Parse response status and size.
  readline(timeout);
  if (! parseReply(F("+HTTPACTION:"), status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), datalen, ',', 2))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_readall(uint16_t *datalen) {
  getReply(F("AT+HTTPREAD"));
  if (! parseReply(F("+HTTPREAD:"), datalen, ',', 0))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_ssl(boolean onoff) {
  return sendCheckReply(F("AT+HTTPSSL="), onoff ? 1 : 0, F("OK"));
}

/********* HTTP HIGH LEVEL FUNCTIONS ***************************/

boolean Adafruit_FONA::HTTP_GET_start(char *url,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen))
    return false;

  Serial.print("Status: "); Serial.println(*status);
  Serial.print("Len: "); Serial.println(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

/*
boolean Adafruit_FONA_3G::HTTP_GET_start(char *ipaddr, char *path, uint16_t port
				      uint16_t *status, uint16_t *datalen){
  char send[100] = "AT+CHTTPACT=\"";
  char *sendp = send + strlen(send);
  memset(sendp, 0, 100 - strlen(send));

  strcpy(sendp, ipaddr);
  sendp+=strlen(ipaddr);
  sendp[0] = '\"';
  sendp++;
  sendp[0] = ',';
  itoa(sendp, port);
  getReply(send, 500);

  return;

  if (! HTTP_setup(url))

    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen))
    return false;

  Serial.print("Status: "); Serial.println(*status);
  Serial.print("Len: "); Serial.println(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}
*/

void Adafruit_FONA::HTTP_GET_end(void) {
  HTTP_term();
}

boolean Adafruit_FONA::HTTP_POST_start(char *url,
              const __FlashStringHelper *contenttype,
              const uint8_t *postdata, uint16_t postdatalen,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  if (! HTTP_para(F("CONTENT"), contenttype)) {
    return false;
  }

  // HTTP POST data
  if (! HTTP_data(postdatalen, 10000))
    return false;
  mySerial->write(postdata, postdatalen);
  if (! expectReply(F("OK")))
    return false;

  // HTTP POST
  if (! HTTP_action(FONA_HTTP_POST, status, datalen))
    return false;

  Serial.print("Status: "); Serial.println(*status);
  Serial.print("Len: "); Serial.println(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_POST_end(void) {
  HTTP_term();
}

void Adafruit_FONA::setUserAgent(const __FlashStringHelper *useragent) {
  this->useragent = useragent;
}

void Adafruit_FONA::setHTTPSRedirect(boolean onoff) {
  httpsredirect = onoff;
}

/********* HTTP HELPERS ****************************************/

boolean Adafruit_FONA::HTTP_setup(char *url) {
  // Handle any pending
  HTTP_term();

  // Initialize and set parameters
  if (! HTTP_init())
    return false;
  if (! HTTP_para(F("CID"), 1))
    return false;
  if (! HTTP_para(F("UA"), useragent))
    return false;
  if (! HTTP_para(F("URL"), url))
    return false;

  // HTTPS redirect
  if (httpsredirect) {
    if (! HTTP_para(F("REDIR"),1))
      return false;

    if (! HTTP_ssl(true))
      return false;
  }

  return true;
}

/********* HELPERS *********************************************/

boolean Adafruit_FONA::expectReply(const __FlashStringHelper *reply,
                                   uint16_t timeout) {
  readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print(F("\t<--- ")); Serial.println(replybuffer);
#endif
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}

/********* LOW LEVEL *******************************************/

inline int Adafruit_FONA::available(void) {
  return mySerial->available();
}

inline size_t Adafruit_FONA::write(uint8_t x) {
  return mySerial->write(x);
}

inline int Adafruit_FONA::read(void) {
  return mySerial->read();
}

inline int Adafruit_FONA::peek(void) {
  return mySerial->peek();
}

inline void Adafruit_FONA::flush() {
  mySerial->flush();
}

void Adafruit_FONA::flushInput() {
    // Read all available serial input to flush pending data.
    uint16_t timeoutloop = 0;
    while (timeoutloop++ < 40) {
        while(available()) {
            read();
            timeoutloop = 0;  // If char was received reset the timer
        }
        delay(1);
    }
}

uint16_t Adafruit_FONA::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replybuffer)-1)) {
    if (mySerial->available()) {
      replybuffer[idx] = mySerial->read();
      idx++;
      b--;
    }
  }
  replybuffer[idx] = 0;

  return idx;
}

uint8_t Adafruit_FONA::readline(uint16_t timeout, boolean multiline) {
  uint16_t replyidx = 0;

  while (timeout--) {
    if (replyidx >= 254) {
      //Serial.println(F("SPACE"));
      break;
    }

    while(mySerial->available()) {
      char c =  mySerial->read();
      if (c == '\r') continue;
      if (c == 0xA) {
        if (replyidx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiline) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
      }
      replybuffer[replyidx] = c;
      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);
      replyidx++;
    }

    if (timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  replybuffer[replyidx] = 0;  // null term
  return replyidx;
}

uint8_t Adafruit_FONA::getReply(char *send, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print("\t---> "); Serial.println(send);
#endif

  mySerial->println(send);

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

uint8_t Adafruit_FONA::getReply(const __FlashStringHelper *send, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> "); Serial.println(send);
#endif

  mySerial->println(send);

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(const __FlashStringHelper *prefix, char *suffix, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> "); Serial.print(prefix); Serial.println(suffix);
#endif

  mySerial->print(prefix);
  mySerial->println(suffix);

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(const __FlashStringHelper *prefix, int32_t suffix, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> "); Serial.print(prefix); Serial.println(suffix, DEC);
#endif

  mySerial->print(prefix);
  mySerial->println(suffix, DEC);

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

// Send prefix, suffix, suffix2, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(const __FlashStringHelper *prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> "); Serial.print(prefix);
  Serial.print(suffix1, DEC); Serial.print(","); Serial.println(suffix2, DEC);
#endif

  mySerial->print(prefix);
  mySerial->print(suffix1);
  mySerial->print(',');
  mySerial->println(suffix2, DEC);

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

// Send prefix, ", suffix, ", and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReplyQuoted(const __FlashStringHelper *prefix, const __FlashStringHelper *suffix, uint16_t timeout) {
  flushInput();

#ifdef ADAFRUIT_FONA_DEBUG
  Serial.print("\t---> "); Serial.print(prefix);
  Serial.print('"'); Serial.print(suffix); Serial.println('"');
#endif

  mySerial->print(prefix);
  mySerial->print('"');
  mySerial->print(suffix);
  mySerial->println('"');

  uint8_t l = readline(timeout);
#ifdef ADAFRUIT_FONA_DEBUG
    Serial.print (F("\t<--- ")); Serial.println(replybuffer);
#endif
  return l;
}

boolean Adafruit_FONA::sendCheckReply(char *send, char *reply, uint16_t timeout) {
  getReply(send, timeout);

/*
  for (uint8_t i=0; i<strlen(replybuffer); i++) {
    Serial.print(replybuffer[i], HEX); Serial.print(" ");
  }
  Serial.println();
  for (uint8_t i=0; i<strlen(reply); i++) {
    Serial.print(reply[i], HEX); Serial.print(" ");
  }
  Serial.println();
  */
  return (strcmp(replybuffer, reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(const __FlashStringHelper *send, const __FlashStringHelper *reply, uint16_t timeout) {
  getReply(send, timeout);
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(const __FlashStringHelper *prefix, char *suffix, const __FlashStringHelper *reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(const __FlashStringHelper *prefix, int32_t suffix, const __FlashStringHelper *reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, suffix2, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(const __FlashStringHelper *prefix, int32_t suffix1, int32_t suffix2, const __FlashStringHelper *reply, uint16_t timeout) {
  getReply(prefix, suffix1, suffix2, timeout);
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, ", suffix, ", and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReplyQuoted(const __FlashStringHelper *prefix, const __FlashStringHelper *suffix, const __FlashStringHelper *reply, uint16_t timeout) {
  getReplyQuoted(prefix, suffix, timeout);
  return (strcmp_P(replybuffer, (prog_char*)reply) == 0);
}


boolean Adafruit_FONA::parseReply(const __FlashStringHelper *toreply,
          uint16_t *v, char divider, uint8_t index) {
  char *p = strstr_P(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=strlen_P((prog_char*)toreply);
  //Serial.println(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //Serial.println(p);

  }
  *v = atoi(p);

  return true;
}

boolean Adafruit_FONA::parseReply(const __FlashStringHelper *toreply,
          char *v, char divider, uint8_t index) {
  uint8_t i=0;
  char *p = strstr_P(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=strlen_P((prog_char*)toreply);

  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  for(i=0; i<strlen(p);i++) {
    if(p[i] == divider)
      break;
    v[i] = p[i];
  }

  v[i] = '\0';

  return true;
}

// Parse a quoted string in the response fields and copy its value (without quotes)
// to the specified character array (v).  Only up to maxlen characters are copied
// into the result buffer, so make sure to pass a large enough buffer to handle the
// response.
boolean Adafruit_FONA::parseReplyQuoted(const __FlashStringHelper *toreply,
          char *v, int maxlen, char divider, uint8_t index) {
  uint8_t i=0, j;
  // Verify response starts with toreply.
  char *p = strstr_P(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=strlen_P((prog_char*)toreply);

  // Find location of desired response field.
  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  // Copy characters from response field into result string.
  for(i=0, j=0; j<maxlen && i<strlen(p); ++i) {
    // Stop if a divier is found.
    if(p[i] == divider)
      break;
    // Skip any quotation marks.
    else if(p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxlen)
    v[j] = '\0';

  return true;
}

boolean Adafruit_FONA::sendParseReply(const __FlashStringHelper *tosend,
				      const __FlashStringHelper *toreply,
				      uint16_t *v, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, v, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


// needed for CBC and others

boolean Adafruit_FONA_3G::sendParseReply(const __FlashStringHelper *tosend,
				      const __FlashStringHelper *toreply,
				      float *f, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, f, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


boolean Adafruit_FONA_3G::parseReply(const __FlashStringHelper *toreply,
          float *f, char divider, uint8_t index) {
  char *p = strstr_P(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=strlen_P((prog_char*)toreply);
  //Serial.println(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //Serial.println(p);

  }
  *f = atof(p);

  return true;
}