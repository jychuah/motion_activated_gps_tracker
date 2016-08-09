#include "application.h"
#include "config.h"

Config config;

void setup() {
    Serial.begin(9600);
    Serial.println("setup");
    config.begin();
}

void loop() {
	delay(5000);
	Serial.println("ping");
}
