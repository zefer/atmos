#include <Arduino.h>
#include <WiFiManager.h>

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered WiFi config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  WiFiManager wifiManager;

  Serial.begin(9600);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
}

void loop() {
}