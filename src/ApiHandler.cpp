#include "ApiHandler.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

ApiHandler::ApiHandler(ConfigManager& configManager)
  : _configManager(configManager) {
}

bool ApiHandler::registerDeviceIfNeeded() {
  DeviceConfig& config = _configManager.getMutableConfig();

  // If we already have a deviceId, we don't need to register.
  if (strlen(config.deviceId) > 0) {
    Serial.println("Device is already registered.");
    return true;
  }

  Serial.println("Device not registered. Attempting registration...");

  // --- Create the JSON payload ---
  JsonDocument doc;
  doc["name"] = config.deviceName;
  doc["type"] = config.deviceType;
  doc["locationHint"] = config.locationHint;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  // --- Perform the HTTP POST request ---
  HTTPClient http;
  String registrationUrl = String(config.serverUrl) + "/api/devices";
  http.begin(registrationUrl);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Sending registration request to: " + registrationUrl);
  Serial.println("Payload: " + jsonPayload);

  int httpCode = http.POST(jsonPayload);

  if (httpCode > 0) {
    String responsePayload = http.getString();
    Serial.printf("Registration response code: %d\n", httpCode);
    Serial.println("Response payload: " + responsePayload);

    if (httpCode == HTTP_CODE_CREATED || httpCode == HTTP_CODE_OK) {
      // --- Parse the response to get the deviceId ---
      JsonDocument responseDoc;
      deserializeJson(responseDoc, responsePayload);
      const char* receivedId = responseDoc["id"];

      if (receivedId) {
        strncpy(config.deviceId, receivedId, sizeof(config.deviceId));
        _configManager.saveConfig(); // Save the newly acquired deviceId
        Serial.println("Successfully registered! New Device ID: " + String(config.deviceId));
        http.end();
        return true;
      } else {
        Serial.println("Registration successful, but no 'id' field in response.");
      }
    }
  } else {
    Serial.printf("Registration failed, HTTP error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return false;
}

bool ApiHandler::sendTelemetry(float temperature, float humidity, float battery) {
  const DeviceConfig& config = _configManager.getConfig();

  // We can't send telemetry without a deviceId.
  if (strlen(config.deviceId) == 0) {
    Serial.println("Cannot send telemetry: deviceId is missing.");
    return false;
  }

  // --- Create the JSON payload ---
  JsonDocument doc;
  doc["deviceId"] = config.deviceId;
  doc["metrics"]["temperature_c"] = temperature;
  doc["metrics"]["humidity_pct"] = humidity;
  doc["metrics"]["battery_pct"] = battery;
  // The "extras" field from the README is optional, so we'll omit it for now.

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  // --- Perform the HTTP POST request ---
  HTTPClient http;
  String ingestUrl = String(config.serverUrl) + "/api/ingest";
  http.begin(ingestUrl);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Sending telemetry to: " + ingestUrl);
  Serial.println("Payload: " + jsonPayload);

  int httpCode = http.POST(jsonPayload);

  if (httpCode > 0 && (httpCode >= 200 && httpCode < 300)) {
    Serial.printf("Telemetry sent successfully, response code: %d\n", httpCode);
    http.end();
    return true;
  } else {
    Serial.printf("Telemetry failed, HTTP error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }
}
