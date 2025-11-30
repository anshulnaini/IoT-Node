#ifndef PORTALMANAGER_H
#define PORTALMANAGER_H

#include <WebServer.h>
#include <DNSServer.h>
#include "ConfigManager.h"

/**
 * @brief Manages the Captive Portal for initial device setup.
 * 
 * When active, this class starts a WiFi Access Point and a web server.
 * Any device connecting to the AP will be redirected to a configuration page
 * where they can enter WiFi credentials and other settings.
 */
class PortalManager {
public:
  /**
   * @brief Construct a new Portal Manager object.
   * @param configManager A reference to the main ConfigManager instance.
   */
  PortalManager(ConfigManager& configManager);

  /**
   * @brief Starts the WiFi AP, DNS, and Web servers.
   */
  void start();

  /**
   * @brief Processes DNS and Web server requests. called repeatedly in a loop.
   */
  void loop();

  /**
   * @brief Stops all portal stuff
   */
  void stop();

  /**
   * @brief Checks if the configuration has been successfully submitted.
   * @return true if the user has submitted the form, false otherwise.
   */
  bool isConfigSaved();

private:
  ConfigManager& _configManager;
  WebServer _server;
  DNSServer _dnsServer;
  bool _configSaved;

  // Web Server Handlers
  void handleRoot();
  void handleSave();
  void handleNotFound();
};

#endif // PORTALMANAGER_H
