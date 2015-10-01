
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_9DOF.h>
#include <Wire.h>

Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

sensors_event_t accelerationEvent;  // roll and pitch
sensors_event_t magnetometerEvent;  // yaw (heading)
sensors_vec_t   orientation;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World");
  initSensors();  
}

void loop() {
  // get roll and pitch, calculate and print
  accel.getEvent(&accelerationEvent);
  if (dof.accelGetOrientation(&accelerationEvent, &orientation)) {
    Serial.print("Roll: ");
    Serial.print(orientation.roll);
    Serial.print(" Pitch: ");
    Serial.print(orientation.pitch);
  }

  // get heading, calculate and print
  mag.getEvent(&magnetometerEvent);
  if (dof.magGetOrientation(SENSOR_AXIS_Z, &magnetometerEvent, &orientation)) {
    Serial.print(" Yaw: ");
    Serial.print(orientation.heading);
  }
  Serial.println("");
  delay(1000);
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
  Serial.println("initSensors exiting");
}
