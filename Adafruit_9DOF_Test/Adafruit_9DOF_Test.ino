#include <Wire.h>
#include <avr/sleep.h>

#define ACCEL_INT1_PIN							2

#define LSM303_ADDRESS_ACCEL					(0x32 >> 1)
#define LSM303_REGISTER_ACCEL_CTRL_REG1_A		0x20
#define LSM303_REGISTER_ACCEL_CTRL_REG3_A		0x22
#define LSM303_REGISTER_ACCEL_INT1_CFG_A		0x30
#define LSM303_REGISTER_ACCEL_INT1_THS_A		0x32
#define LSM303_REGISTER_ACCEL_INT1_DURATION_A	0x33
#define SENSITIVITY_PEAK_VOLTAGE				2.45

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello World");
//  initSensors();  
//  initInterrupt();
  pinMode(A0, INPUT);
}

void initInterrupt() {
  pinMode(ACCEL_INT1_PIN, INPUT);
  sleep_enable();
  Serial.println("Installing Interrupt");
  attachInterrupt(digitalPinToInterrupt(ACCEL_INT1_PIN), accelerometerISR, CHANGE); 
  Serial.println("Going to sleep");
  delay(1000);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_bod_disable();
  sei();
  sleep_cpu();
  
  // wake event
  sleep_disable();
}

void loop() {
	Serial.println(getSensitivity());
	delay(1000);
}

uint8_t getSensitivity() {
	double voltage = 5.0 * (analogRead(A0) / 1023.0);
	if (voltage > SENSITIVITY_PEAK_VOLTAGE) {
		voltage = SENSITIVITY_PEAK_VOLTAGE;
	}
	return (uint8_t)((voltage / SENSITIVITY_PEAK_VOLTAGE) * 127);
}

void accelerometerISR() {
	sleep_disable();
  detachInterrupt(digitalPinToInterrupt(ACCEL_INT1_PIN));
  Serial.println("Accelerometer Interrupt");

}
void initSensors()
{
	Serial.println("Initializing Sensors");
	Wire.begin();

	uint8_t cfg1_value = 0x5F;   // \100 hODR cycle, low power, 3 axis

	// Enable the accelerometer (100Hz)
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, cfg1_value);
	// AOI interrupt enable -- position interrupt enabled on LIN1
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG3_A, 0x60);
	// INT1 Configuration -- OR movement recognition, X Y Z High event
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_CFG_A, 0x2A);
	// INT1 Threshold -- absolute value of movement greater than 16 milli-Gs
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_THS_A, 0x4F);
	// INT1 Duration -- movement must be for more than 1 ODR cycle... maybe 10ms
	write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_DURATION_A, 0x01);


	// LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check
	// if we are connected or not
	uint8_t reg1_a = read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A);
	if (reg1_a != cfg1_value)
	{
		Serial.println("Error initializing LSM303");
	}
	else {
		Serial.println("Done initializing LSM303");
	}
}

void write8(byte address, byte reg, byte value)
{
	Wire.beginTransmission(address);
#if ARDUINO >= 100
	Wire.write((uint8_t)reg);
	Wire.write((uint8_t)value);
#else
	Wire.send(reg);
	Wire.send(value);
#endif
	Wire.endTransmission();
}

byte read8(byte address, byte reg)
{
	byte value;

	Wire.beginTransmission(address);
#if ARDUINO >= 100
	Wire.write((uint8_t)reg);
#else
	Wire.send(reg);
#endif
	Wire.endTransmission();
	Wire.requestFrom(address, (byte)1);
#if ARDUINO >= 100
	value = Wire.read();
#else
	value = Wire.receive();
#endif  
	Wire.endTransmission();

	return value;
}
