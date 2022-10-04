#include <Arduino.h>
#include <PubSubClient.h>

struct Measurement {
  const char *name;
  float lastValue;
  unsigned long lastMeasuredAt;

  Measurement(const char *name):
    name(name),
    lastMeasuredAt(0)
    { };

  void record(float value);
  void publish(HardwareSerial * serial);
  void publish(String &json);
  void publish(PubSubClient * client, char * mqttPrefix);
  void broadcast(HardwareSerial * serial, char * formattedValue);
  void broadcast(PubSubClient * client, char * mqttPrefix, char * formattedValue);
};
