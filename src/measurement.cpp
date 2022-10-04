#include <Arduino.h>
#include <measurement.h>
#include <PubSubClient.h>

void Measurement::record(float value) {
  lastValue = value;
  lastMeasuredAt = millis();
}

void Measurement::publish(HardwareSerial * serial) {
  char formattedValue[256];
  sprintf(formattedValue, "%.2f", lastValue);
  broadcast(serial, formattedValue);
}

void Measurement::broadcast(HardwareSerial * serial, char * formattedValue) {
  serial->print(name);
  serial->print(": ");
  serial->println(formattedValue);
}

void Measurement::publish(String &json) {
  json += "{\"name\": \"";
  json += name;
  json += "\", \"value\": ";
  json += lastValue;
  json += ", \"last_measured_at\": ";
  json += lastMeasuredAt;
  json += "}, ";
}

void Measurement::publish(PubSubClient * client, char * mqttPrefix) {
  char formattedValue[256];
  sprintf(formattedValue, "%.2f", lastValue);
  broadcast(client, mqttPrefix, formattedValue);
}

void Measurement::broadcast(PubSubClient * client, char * mqttPrefix, char * formattedValue) {
  // TODO: consider setting this on instantiation to reduce overhead.
  char topic[256];
  sprintf(topic, "%s/%s", mqttPrefix, name);
  client->publish(topic, formattedValue);
}
