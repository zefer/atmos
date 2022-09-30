#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <measurement.h>

Preferences preferences;
WebServer server(80);
Adafruit_BME280 bme;

Measurement temperatureMeasurement("temperature");
Measurement humidityMeasurement("humidity");
Measurement pressureMeasurement("pressure");

const char APP_NAME[] = "atmos";
// How many milliseconds between taking the sensor readings.
const long POLL_DELAY = 1000;

// MQTT server config.
char defaultMqttServer[40] = "";
char defaultMqttPort[6] = "1833";
char defaultMqttNodeName[40] = "atmos-office";
char defaultMqttPrefix[40] = "atmos";
String mqttServer;
String mqttPort;
String mqttNodeName;
String mqttPrefix;

bool shouldSaveConfig = false;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered WiFi config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback () {
  Serial.println("setting shouldSaveConfig to true");
  shouldSaveConfig = true;
}

void webHandleNotFound() {
  server.send(404, "application/json", "{\"message\":\"Not found\"}");
}

void webHandleStatus() {
  String json;
  json.reserve(1024);
  json += "{\"uptime\": ";
  json += millis();
  json += ", \"heap_free\": ";
  json += ESP.getFreeHeap();

  json += ", \"mqtt_config\": {";
  json += "\"server\": \"";
  json += mqttServer;
  json += "\", \"port\": ";
  json += mqttPort;
  json += ", \"node\": \"";
  json += mqttNodeName;
  json += "\", \"prefix\": \"";
  json += mqttPrefix;
  json += "\"}}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// Forget all saved preferences (WiFi & MQTT) and reboot.
void webHandleReset() {
  Serial.println("Resetting.");
  server.send(202);

  // Clear the custom config vars.
  preferences.begin(APP_NAME, false);
  preferences.clear();
  preferences.end();

  // Clear the WiFi connection credentials.
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  ESP.restart();
}

// Simply reboot the system.
void webHandleReboot() {
  Serial.println("Rebooting.");
  server.send(202);
  ESP.restart();
}

void setup() {
  WiFiManager wifiManager;

  preferences.begin(APP_NAME, false);
  Serial.begin(9600);

  // Allow the user to configure MQTT params on the same UI as the WiFi.
  WiFiManagerParameter mqttServerField("server", "mqtt server", defaultMqttServer, 40);
  WiFiManagerParameter mqttPortField("port", "mqtt port", defaultMqttPort, 6);
  WiFiManagerParameter mqttNodeField("nodename", "mqtt node name", defaultMqttNodeName, 40);
  WiFiManagerParameter mqttPrefixField("prefix", "mqtt prefix", defaultMqttPrefix, 40);
  wifiManager.addParameter(&mqttServerField);
  wifiManager.addParameter(&mqttPortField);
  wifiManager.addParameter(&mqttNodeField);
  wifiManager.addParameter(&mqttPrefixField);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");


  if(shouldSaveConfig) {
    Serial.println("Writing MQTT config vars");
    char server[40];
    char port[6];
    char node[40];
    char prefix[40];
    strcpy(server, mqttServerField.getValue());
    strcpy(port, mqttPortField.getValue());
    strcpy(node, mqttNodeField.getValue());
    strcpy(prefix, mqttPrefixField.getValue());
    // Persist the user-input MQTT params to flash storage.
    preferences.putString("mqttServer", server);
    preferences.putString("mqttPort", port);
    preferences.putString("mqttNodeName", node);
    preferences.putString("mqttPrefix", prefix);
  }

  // load MQTT config vars from flash storage.
  Serial.println("Reading MQTT config vars");
  mqttServer = preferences.getString("mqttServer", defaultMqttServer);
  mqttPort = preferences.getString("mqttPort", defaultMqttPort).toInt();
  mqttNodeName = preferences.getString("mqttNodeName", defaultMqttNodeName);
  mqttPrefix = preferences.getString("mqttPrefix", defaultMqttPrefix);
  preferences.end();
  Serial.println("using MQTT prefs...");
  Serial.println(mqttServer);
  Serial.println(mqttPort);
  Serial.println(mqttNodeName);
  Serial.println(mqttPrefix);

  server.on("/status", HTTP_GET, webHandleStatus);
  server.on("/reboot", HTTP_PUT, webHandleReboot);
  server.on("/reset", HTTP_PUT, webHandleReset);
  server.onNotFound(webHandleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  if (!bme.begin(0x76)) {
    Serial.println("Could not find the BME280 sensor");
    while (1);
  }
}

void readBmeSensor(void) {
  static unsigned long lastMeasurementMillis = 0;

  if(millis() - lastMeasurementMillis > POLL_DELAY) {
    lastMeasurementMillis = millis();

    // Temperature in degrees celcius.
    temperatureMeasurement.record(bme.readTemperature());
    // Relative humidity in %.
    humidityMeasurement.record(bme.readHumidity());
    // Barometric pressure in hPa.
    pressureMeasurement.record(bme.readPressure() / 100.0F);

    temperatureMeasurement.publish(&Serial);
    humidityMeasurement.publish(&Serial);
    pressureMeasurement.publish(&Serial);

    Serial.println();
  }
}

void loop() {
  server.handleClient();
  readBmeSensor();
}
