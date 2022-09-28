#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>

WebServer server(80);

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered WiFi config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void handleNotFound() {
  server.send(404, "application/json", "{\"message\":\"Not found\"}");
}

void handleStatus() {
  String json;
  json.reserve(1024);
  json += "{\"current_time\": ";
  json += millis();
  json += ", \"heap_free\":";
  json += ESP.getFreeHeap();
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
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

  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
