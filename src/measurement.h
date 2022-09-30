#include <Arduino.h>

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
  void broadcast(HardwareSerial * serial, char * formattedValue);
};
