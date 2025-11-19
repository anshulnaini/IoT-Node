# IoT Node Project Plan

This document outlines the development plan for the ESP32-based IoT Node. The project is broken down into logical phases to ensure each component is working before integrating it into the final system.

---

### Phase 1: Core Component Setup & Testing

**Goal:** Verify that each hardware component can be controlled by the ESP32-C3. Each step should result in a small, standalone test sketch.

1.  **Basic Board Setup:**  (DONE)
    *   Configure `platformio.ini` for the ESP32-C3 board (`esp32-c3-devkitm-1` or similar).
    *   Create a "Hello, World" sketch that prints to the Serial monitor to confirm toolchain and upload functionality.

2.  **OLED Display Integration:** (DONE)
    *   Use the existing `OLEDHandler` class.
    *   Confirm it works by displaying a test message.

3.  **I2C Sensor Integration:**
    *   Choose and add a library for the I2C temperature/humidity sensor (e.g., Adafruit SHT31, AHTX0).
    *   Write a test sketch to read temperature and humidity values and print them to the Serial monitor.

4.  **Pushbutton Integration:**
    *   Add the `mathertel/OneButton` library to handle debouncing, multi-press, and long-press events.
    *   Write a test sketch to detect and print single, double, triple, and long-press events to the Serial monitor.

5.  **Power Management Test:**
    *   Write a sketch to control two GPIO pins connected to LEDs (as stand-ins for the transistors).
    *   Verify that you can turn the "peripherals" on and off programmatically.

---

### Phase 2: Configuration, WiFi, and NV Memory

**Goal:** Implement the device setup and configuration mechanism.

1.  **Non-Volatile (NV) Memory:**
    *   Use the ESP32 `Preferences` library.
    *   Create a `ConfigManager` class to handle saving, loading, and clearing a `struct` of configuration data (WiFi credentials, server details, device name, etc.).
    *   Test that configuration persists after a power cycle and can be cleared.

2.  **Captive Portal & Web Server:**
    *   Use the `WebServer` and `DNSServer` libraries (or a higher-level library like `WiFiManager`) to create a Soft AP.
    *   Implement a captive portal that redirects clients to a configuration web page.
    *   Create the HTML/CSS/JS for the configuration form with all the required fields.

3.  **Integration:**
    *   Combine NV Memory and the captive portal.
    *   On boot, the device should check `ConfigManager` if it's configured.
        *   **If NOT configured:** Start the captive portal. When the form is submitted, save the data using `ConfigManager` and reboot.
        *   **If configured:** Proceed to connect to the saved WiFi network.

---

### Phase 3: API Communication

**Goal:** Send data to the remote web server.

1.  **WiFi Connection:**
    *   Using the loaded configuration, connect to the user's WiFi network.
    *   Display the connection status (and IP address on success) on the OLED screen using `OLEDHandler`.

2.  **HTTP Client:**
    *   Use the built-in `HTTPClient` library.
    *   Use `bblanchon/ArduinoJson` to create the JSON payload matching the Prisma schema.
    *   Write a function to send telemetry data to the server.
    *   Write a function to request a UUID from the server if one is not already configured.

---

### Phase 4: State Machine, Sleep, and Interrupts

**Goal:** Implement the main application logic and power-saving features.

1.  **Finite State Machine (FSM):**
    *   Define the main states: `SETUP`, `CONNECTING`, `RUNNING_TELEMETRY`, `INFO_DISPLAY`, `FACTORY_RESET`.
    *   The `loop()` function will primarily manage transitions between these states.

2.  **Deep Sleep & Power Management:**
    *   Create a `PowerManager` class to handle `peripherals_on()`, `peripherals_off()`, and `enterDeepSleep()`.
    *   Use `esp_deep_sleep_start()` to put the device to sleep for the configured sample period.

3.  **Wake-up Logic:**
    *   Use `RTC_DATA_ATTR` to store a variable that indicates the reason for waking up (e.g., timer, button press).
    *   Configure a GPIO pin connected to the button as an external wake-up source.
    *   In the `setup()` function, check the wake-up reason to determine the correct behavior (e.g., send telemetry vs. show info screen).

---

### Phase 5: Final Integration & Refinement

**Goal:** Combine all features into a robust, final application.

1.  **Full Button Logic:**
    *   Integrate the `OneButton` callbacks with the main FSM.
    *   Implement the full logic for all states:
        *   **Sleep:** Wake to show info.
        *   **Info Display:** Single press (sleep), double (send telemetry), triple (enter setup).
        *   **Any (except sleep):** Long press for factory reset.

2.  **OLED Handler Expansion:**
    *   Flesh out the `OLEDHandler` to display all necessary information: status messages, device info, sensor readings, etc., corresponding to the current FSM state. This will involve adding new functions to the class.

3.  **Battery Measurement:**
    *   Use the ESP32's ADC to read the battery voltage.
    *   Create a function to convert the voltage reading to a battery percentage (0-100%).
    *   Add `battery_pct` to the telemetry payload.

4.  **End-to-End Testing:**
    *   Perform a full user-story test:
        1.  Power on for the first time -> Verify captive portal.
        2.  Submit config -> Verify connection and UUID retrieval.
        3.  Verify it goes to sleep and wakes to send telemetry.
        4.  Test all button press scenarios in different states.
        5.  Test the factory reset functionality.
        6.  Verify all information is displayed correctly on the OLED.
