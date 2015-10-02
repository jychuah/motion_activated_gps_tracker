
#include "Adafruit_LSM303_U.h"
#include "Adafruit_L3GD20_U.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_9DOF.h"
#include <Wire.h>

Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

sensors_event_t accelerationEvent;  // roll and pitch
sensors_event_t magnetometerEvent;  // yaw (heading)
sensors_vec_t   orientation;
int a0val = -100;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World");
  initSensors();  
  pinMode(A0, INPUT);
  
}

bool lastInterrupt = false;

void loop() {
	/*
  int newVal = analogRead(A0);
  if (abs(newVal - a0val) > 10) {
    a0val = newVal;
    Serial.print("A0: ");
    Serial.println(5.0 * (newVal / 1023.0));
	if (readOrientation()) {
		Serial.print("orientation: ");
		Serial.print(orientation.pitch);
		Serial.print(", ");
		Serial.print(orientation.roll);
		Serial.print(", ");
		Serial.print(orientation.heading);
		Serial.println("");
	}
	else {
		Serial.println("Could not read orientation");
	}
	
	
  }*/
	bool currentInterrupt = accel.interruptActive();
	if (currentInterrupt != lastInterrupt) {
		if (!lastInterrupt) {
			Serial.println("*** AGITATED ***");
		}
		else {
			Serial.println("calm");
		}
		lastInterrupt = currentInterrupt;
	}
}

bool readOrientation() {
  accel.getEvent(&accelerationEvent);
  mag.getEvent(&magnetometerEvent);
  return dof.fusionGetOrientation(&accelerationEvent, &magnetometerEvent, &orientation);
}

void initSensors()
{
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while(1);
  } else {
    Serial.println("LSM303 accelerometer initialized");
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  } else {
    Serial.println("LSM303 magnetometer initialized");
  }
}
