#ifndef __CONFIG_CPP__
#define __CONFIG_CPP__

#include "config.h"
#include "application.h"

Config::Config() {

}

void Config::begin() {
	Serial.println("beginning config");
    Particle.subscribe("hook-response/config_get", &Config::parseValue, this);
    Particle.publish("config_get", PRIVATE);
}

void Config::parseValue(const char *event, const char *data) {
	Serial.println("Callback");
	String d = String(data);
	Serial.println(data);
}

#endif
