
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
int a0val = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World");
  initSensors();  
  setInterrupts();
  pinMode(A0, INPUT);
  
}

void loop() {
  int newVal = analogRead(A0);
  if (abs(newVal - a0val) > 5) {
    a0val = newVal;
    Serial.print("A0: ");
    Serial.println(a0val);
  }
}

bool readOrientation() {
  accel.getEvent(&accelerationEvent);
  mag.getEvent(&magnetometerEvent);
  return dof.fusionGetOrientation(&accelerationEvent, &magnetometerEvent, &orientation);
}

byte i2c_read(byte address, byte reg) {
  byte value;

  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}

void i2c_write(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}



void setInterrupts() {
  
  // Enable 6D motion detection interrupt  
  i2c_write(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT2_CFG_A, 0x7f);

  // Set threshold
  i2c_write(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT2_THS_A, 0x0f);

  // Set duration
  i2c_write(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT2_DURATION_A, 0x0f);
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
