#ifndef __CONFIG_CPP__
#define __CONFIG_CPP__

#include "config.h"
#include "application.h"

Config::Config() {

}

void Config::begin() {
	Serial.println("beginning config");
	
    Particle.subscribe("hook-response/config_get", &Config::parse_config_get, this, MY_DEVICES);
    Particle.subscribe("child_changed", &Config::parse_child_changed, this, MY_DEVICES);
    Particle.publish("config_get", PRIVATE);
    Particle.publish("config_changed", PRIVATE);
}

void Config::parse_child_changed(const char *event, const char *data) {
    Serial.println("child_changed");
    Serial.println(data);
}

void Config::parse_config_get(const char *event, const char *data) {
	Serial.println("config_get");
	String d = String(data).trim();

	// Test fixture
	// String d = String("{\"key1\":\"value1\",\"key2\":\"value2\"}");

	// Strip beginning and ending curly braces
	if (d.startsWith("{") && d.endsWith("}")) {
		d = d.substring(1, d.length() - 1);
	}

	// Search for key pairs
	int comma = d.indexOf(",");
	while (comma > -1) {
		String pair = d.substring(0, comma);
		d = d.substring(comma + 1);
		parse_pair(pair);
		comma = d.indexOf(",");
	}
	// Process last pair
	parse_pair(d);

}

void Config::parse_pair(String pair) {
	int colon = pair.indexOf(":");
	String key = pair.substring(0, colon);
	String value = pair.substring(colon + 1);
	Serial.print("key: ");
	Serial.println(key);
	Serial.print("value: ");
	Serial.println(value);
}

#endif
