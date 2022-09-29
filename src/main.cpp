#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Preferences.h>

Preferences preferences;
WebServer server(80);
WiFiClient espClient;

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

  preferences.begin("atmos", false);
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
  mqttPort = preferences.getString("mqttPort", defaultMqttPort);
  mqttNodeName = preferences.getString("mqttNodeName", defaultMqttNodeName);
  mqttPrefix = preferences.getString("mqttPrefix", defaultMqttPrefix);
  preferences.end();
  Serial.println("using MQTT prefs...");
  Serial.println(mqttServer);
  Serial.println(mqttPort);
  Serial.println(mqttNodeName);
  Serial.println(mqttPrefix);

  server.on("/status", webHandleStatus);
  server.onNotFound(webHandleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
