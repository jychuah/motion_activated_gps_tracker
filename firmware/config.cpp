#ifndef __CONFIG_CPP__
#define __CONFIG_CPP__

#include "config.h"
#include "application.h"

Config::Config() {

}

void Config::begin() {
    Particle.subscribe("hook-response/config_get", &Config::parse_config_get, this, MY_DEVICES);
    Particle.subscribe("child_changed", &Config::parse_child_changed, this, MY_DEVICES);
    Particle.publish("config_get", PRIVATE);
    Particle.publish("config_changed", PRIVATE);
}

void Config::parse_child_changed(const char *event, const char *data) {
    String d = String(data).trim();
    d = d.substring(1, d.length() - 1);
    parse_pair(d);
}

void Config::parse_config_get(const char *event, const char *data) {
	String d = String(data).trim();

	// Test fixture
	// String d = String("{\"key1\":\"value1\",\"key2\":\"value2\"}");
    d = strip_surrounding(d, '{', '}');

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

String Config::strip_surrounding(String input, char startswith, char endswith) {
    String result = String(input);
    if (result.startsWith(String(startswith))) {
        result = result.substring(1, result.length());
    }
    if (result.endsWith(String(endswith))) {
        result = result.substring(0, result.length() - 1);
    }
    return result;
}

void Config::parse_pair(String pair) {
	int colon = pair.indexOf(":");
	String key = pair.substring(0, colon);
	String value = pair.substring(colon + 1);
    key = strip_surrounding(key, '"', '"');
    value = strip_surrounding(value, '"', '"');
	Serial.print("key: ");
	Serial.println(key);
	Serial.print("value: ");
	Serial.println(value);
}

#endif
