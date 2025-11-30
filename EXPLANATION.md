## Project Overview: IoT Telemetry Node

This project implements a low-power IoT telemetry node using an ESP32-C3 microcontroller. Its primary function is to collect environmental data (temperature and humidity), report it to a central server, and operate efficiently on battery power.

The core design principles are:
*   **Modularity:** The code is organized into distinct classes (handlers, managers) for different functionalities.
*   **Power Efficiency:** Utilizes ESP32's deep sleep mode and switches off peripherals when not in use.
*   **Responsiveness:** Employs a non-blocking Finite State Machine (FSM) to ensure the device can always respond to user input (button presses).
*   **Easy Configuration:** Provides a captive web portal for initial setup.

The device's operation follows a general cycle: **Boot -> (Setup if unconfigured) -> Connect to WiFi -> Send Telemetry -> Deep Sleep -> Wakeup -> Repeat.**

---

## `main.cpp`: The Orchestrator

`main.cpp` is the heart of the application. It doesn't implement specific hardware logic itself but orchestrates all the other components using a **Finite State Machine (FSM)**.

*   **`setup()` function:**
    *   This runs once on boot.
    *   It initializes the Serial communication, powers on peripherals (OLED, sensor) via the `PowerManager`, initializes the `OLEDHandler`, `ConfigManager`, and `ButtonHandler`.
    *   Crucially, it calls `checkWakeupReason()` to determine why the ESP32 woke up (cold boot, timer, or button press) and sets the initial `currentState` of the FSM accordingly.

*   **`loop()` function:**
    *   This function runs continuously and is the core of the FSM.
    *   It's designed to be **non-blocking**, meaning it never uses `delay()` (except for terminal events like factory reset or system restart) and executes very quickly on each iteration. This ensures the device remains responsive to button presses.
    *   It first calls `buttonHandler.tick()` and `buttonHandler.getEvent()` to process any button interactions.
    *   It then checks for a global `EV_LONG_PRESS` event to trigger a factory reset.
    *   Finally, a `switch` statement executes logic specific to the `currentState`.

*   **`enum DeviceState`:** Defines the different operational modes of the device:
    *   `STATE_BOOT`: The very first state after power-on or reset. It decides whether to go to setup or connect to WiFi.
    *   `STATE_INFO_DISPLAY`: Entered when the device wakes from deep sleep due to a button press. It displays device information and sensor readings.
    *   `STATE_SETUP_START`, `STATE_SETUP_RUNNING`, `STATE_SETUP_COMPLETE`: These states manage the captive web portal for initial configuration.
    *   `STATE_CONNECTING_WIFI`: Handles connecting to the configured WiFi network.
    *   `STATE_TELEMETRY_SEND`: Manages device registration (if needed) and sends sensor data to the backend server.
    *   `STATE_TASK_COMPLETE`: A temporary state that waits for a few seconds (using a non-blocking timer) to display a status message on the OLED before transitioning to deep sleep.
    *   `STATE_DEEP_SLEEP`: The state where the device prepares for and enters ESP32's deep sleep mode.

*   **`checkWakeupReason()` function:**
    *   Called in `setup()`.
    *   Uses `esp_sleep_get_wakeup_cause()` to determine if the device woke up due to a timer, an external GPIO (button), or a cold boot (undefined).
    *   Sets the `currentState` to `STATE_CONNECTING_WIFI` (for timer wakeup), `STATE_INFO_DISPLAY` (for button wakeup), or `STATE_BOOT` (for cold boot).

*   **`connectToWiFi()` function:**
    *   Attempts to connect to the WiFi network using credentials from the `ConfigManager`.
    *   Includes a timeout and provides visual feedback on the OLED.
    *   It's currently a blocking function (uses a `while` loop) but includes `buttonHandler.tick()` and `getEvent()` calls to maintain responsiveness for factory reset during connection attempts.

---

## Header Files: Modular Components

Each header file (`.h`) defines the interface for a specific module, and its corresponding `.cpp` file contains the implementation.

### `ConfigManager.h` / `ConfigManager.cpp`
*   **Purpose:** Manages the device's persistent configuration data. This includes WiFi credentials, server URL, device name, device ID (assigned by the server), and sleep interval. It uses the ESP32's Non-Volatile Storage (NVS) to save settings across reboots.
*   **Key Classes/Functions:**
    *   `struct DeviceConfig`: Holds all configuration parameters.
    *   `loadConfig()`: Reads settings from NVS into memory.
    *   `saveConfig()`: Writes current settings from memory to NVS.
    *   `clearConfig()`: Erases all saved settings.
    *   `isConfigured()`: Checks if the device has been set up previously.
*   **Interaction:** `main.cpp` uses it on boot to load settings and after setup to save new ones. `ApiHandler` and `PowerManager` retrieve configuration details from it.

### `PortalManager.h` / `PortalManager.cpp`
*   **Purpose:** Implements a captive web portal (Soft AP) that allows users to configure the device's WiFi and server settings from a phone or computer.
*   **Key Classes/Functions:**
    *   `start()`: Initializes the Soft AP and web server.
    *   `loop()`: Must be called repeatedly to handle web server requests.
    *   `isConfigSaved()`: Indicates if the user has submitted the configuration form.
    *   `stop()`: Shuts down the Soft AP and web server.
*   **Interaction:** Used by `main.cpp` in the `STATE_SETUP_START` and `STATE_SETUP_RUNNING` states to guide the user through the initial setup process.

### `ButtonHandler.h` / `ButtonHandler.cpp`
*   **Purpose:** Provides a robust way to handle various button press events (single click, double click, triple click, long press) from a single physical button, including debouncing. It wraps the `OneButton` library.
*   **Key Classes/Functions:**
    *   `begin()`: Initializes the button.
    *   `tick()`: **Must be called frequently** in `loop()` to process button state changes.
    *   `getEvent()`: Returns the last detected `ButtonEvent` and consumes it (returns `EV_NONE` until a new event occurs).
    *   `enum ButtonEvent`: Defines the types of button events.
*   **Interaction:** `main.cpp` calls `tick()` in every `loop()` iteration and `getEvent()` to check for user input, which then drives state transitions or actions.

### `OLEDHandler.h` / `OLEDHandler.cpp`
*   **Purpose:** Manages the 128x64 I2C OLED display to provide visual feedback and status information to the user.
*   **Key Classes/Functions:**
    *   `initializeOLED()`: Sets up the display.
    *   `displayText(const char* text)`: Displays a single line of text, centered.
    *   `displayInfo(const char* deviceName, const char* deviceId, const char* ipAddress, float temp, float humidity)`: Displays a formatted screen with device details and sensor readings.
    *   `clearDisplay()`: Clears the OLED screen.
*   **Interaction:** `main.cpp` uses this to show boot messages, setup instructions, connection status, sensor data, and other operational feedback.

### `ApiHandler.h` / `ApiHandler.cpp`
*   **Purpose:** Handles all HTTP communication with the backend server, including device registration and telemetry data submission. It uses the `HTTPClient` and `ArduinoJson` libraries.
*   **Key Classes/Functions:**
    *   `registerDeviceIfNeeded()`: Checks if the device has a `deviceId`. If not, it sends a `POST` request to `/api/devices` to register and stores the received ID.
    *   `sendTelemetry(float temperature, float humidity, float battery)`: Constructs a JSON payload with sensor data and sends a `POST` request to `/api/ingest`.
*   **Interaction:** `main.cpp` calls these methods in the `STATE_TELEMETRY_SEND` state to interact with the cloud platform.

### `PowerManager.h` / `PowerManager.cpp`
*   **Purpose:** Manages the device's power states, specifically controlling deep sleep and switching power to peripherals (OLED and sensor) via transistors.
*   **Key Classes/Functions:**
    *   `peripherals_on()`: Turns on the power to the OLED and sensor.
    *   `peripherals_off()`: Turns off the power to the OLED and sensor.
    *   `enterDeepSleep(uint32_t sleepDurationSeconds)`: Configures the ESP32 for timer and GPIO wakeup, then puts the device into deep sleep using `esp_deep_sleep_start()`.
*   **Interaction:** `main.cpp` calls `peripherals_on()` early in `setup()`, `peripherals_off()` just before deep sleep, and `enterDeepSleep()` in the `STATE_DEEP_SLEEP` state.

### `SensorHandler.h` / `SensorHandler.cpp`
*   **Purpose:** Interfaces with the AHT10 temperature and humidity sensor, abstracting the sensor-specific communication. It uses the `Adafruit_AHTX0` library.
*   **Key Classes/Functions:**
    *   `begin()`: Initializes communication with the AHT10 sensor.
    *   `readTemperature()`: Reads and returns the current temperature.
    *   `readHumidity()`: Reads and returns the current relative humidity.
*   **Interaction:** `main.cpp` calls `begin()` in `setup()` and then `readTemperature()` and `readHumidity()` in `STATE_TELEMETRY_SEND` (and `STATE_INFO_DISPLAY`) to get the environmental data.