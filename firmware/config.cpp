#ifndef __CONFIG_CPP__
#define __CONFIG_CPP__

#include "config.h"
#include "application.h"

Config::Config() {

}

void Config::begin() {
	Serial.println("beginning config");
	/*
    Particle.subscribe("hook-response/config_get", &Config::parse_config_get, this, MY_DEVICES);
    Particle.publish("config_get", PRIVATE);
    */
    parse_config_get(NULL, NULL);
}

void Config::parse_config_get(const char *event, const char *data) {
	Serial.println("Callback");
//	String d = String(data);
	String d = String("{\"key1\":\"value1\",\"key2\":\"value2\"}");
	Serial.println(d);
}

#endif
