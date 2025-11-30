#include "PortalManager.h"
#include <WiFi.h>

// config  page
const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>IoT Node Setup</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background-color: #f2f2f2; color: #333; margin: 0; padding: 20px; }
        .container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #007aff; text-align: center; }
        label { display: block; margin-top: 15px; font-weight: bold; }
        input[type=text], input[type=password] { width: calc(100% - 22px); padding: 10px; margin-top: 5px; border: 1px solid #ccc; border-radius: 4px; }
        input[type=submit] { background-color: #007aff; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px; margin-top: 20px; }
        input[type=submit]:hover { background-color: #0056b3; }
        .group { border-top: 1px solid #eee; padding-top: 10px; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>IoT Node Setup</h1>
        <form action="/save" method="POST">
            <div class="group">
                <label for="ssid">WiFi Network (SSID)</label>
                <input type="text" id="ssid" name="ssid" required>
                <label for="pass">WiFi Password</label>
                <input type="password" id="pass" name="pass">
            </div>
            <div class="group">
                <label for="server">Server URL</label>
                <input type="text" id="server" name="server" placeholder="http://192.168.1.100:4000/api" required>
            </div>
            <div class="group">
                <label for="name">Device Name</label>
                <input type="text" id="name" name="name" placeholder="e.g., Living Room Sensor" required>
                <label for="type">Device Type</label>
                <select id="type" name="type">
                    <option value="Temp/Humidity">Temp/Humidity</option>
                </select>
                <label for="location">Location Hint</label>
                <input type="text" id="location" name="location" placeholder="e.g., On the bookshelf">
            </div>
            <div class="group">
                <label for="interval">Sleep Interval (seconds)</label>
                <input type="text" id="interval" name="interval" value="300" required>
            </div>
            <input type="submit" value="Save Configuration">
        </form>
    </div>
</body>
</html>
)rawliteral";

//  AP Settings 
const char* AP_SSID = "IoT-Node-Setup";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

PortalManager::PortalManager(ConfigManager& configManager)
  : _configManager(configManager), _server(80), _configSaved(false) {
}

void PortalManager::start() {
  Serial.println("Starting Portal Manager...");
  WiFi.softAP(AP_SSID);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  _dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("DNS server started.");

  _server.on("/", HTTP_GET, [this]() { this->handleRoot(); });
  _server.on("/save", HTTP_POST, [this]() { this->handleSave(); });
  _server.onNotFound([this]() { this->handleNotFound(); });
  _server.begin();
  Serial.println("Web server started. AP SSID: " + String(AP_SSID));
}

void PortalManager::loop() {
  _dnsServer.processNextRequest();
  _server.handleClient();
}

void PortalManager::stop() {
  _server.stop();
  _dnsServer.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("Portal Manager stopped.");
}

bool PortalManager::isConfigSaved() {
  return _configSaved;
}

// handlers

void PortalManager::handleRoot() {
  _server.send(200, "text/html", CONFIG_PAGE);
}

void PortalManager::handleSave() {
  Serial.println("Handling save request...");
  DeviceConfig& config = _configManager.getMutableConfig();

  strncpy(config.wifiSSID, _server.arg("ssid").c_str(), sizeof(config.wifiSSID));
  strncpy(config.wifiPassword, _server.arg("pass").c_str(), sizeof(config.wifiPassword));
  strncpy(config.serverUrl, _server.arg("server").c_str(), sizeof(config.serverUrl));
  strncpy(config.deviceName, _server.arg("name").c_str(), sizeof(config.deviceName));
  strncpy(config.deviceType, _server.arg("type").c_str(), sizeof(config.deviceType));
  strncpy(config.locationHint, _server.arg("location").c_str(), sizeof(config.locationHint));
  config.sleepIntervalSeconds = _server.arg("interval").toInt();
  config.configured = true;

  _configManager.saveConfig();
  
  String response = "<h1>Configuration Saved!</h1><p>The device will now restart and try to connect to your WiFi.</p>";
  _server.send(200, "text/html", response);

  _configSaved = true;
  Serial.println("Configuration saved.");
}

void PortalManager::handleNotFound() {
  _server.sendHeader("Location", String("http://") + apIP.toString(), true);
  _server.send(302, "text/plain", "");
}
