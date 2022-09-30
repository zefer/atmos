#include <Arduino.h>
#include <measurement.h>

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
