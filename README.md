Overview

My project is an IoT sensor node that can be used to measure temperature and humidity in its environment. Its primary use case is for smaller family owned restaurants since they have to follow health regulations to keep logs of their cold store environments. This sensor automates the logging and sends over the data to a web server.

How to Use

When connected to power for the first time the device will enter a setup mode where it will create a soft AP that you can connect to. When you are connected, it opens up a captive portal automatically that you can use to input data such as the name, location, type, server details, wifi ssid and password, and telemetry period. Once you input that it will store all of this into non volatile memory so that if it loses power again you don't have to set it up again (every time it boots it checks to see if it already has been configured). Then it will attempt to connect to the wifi and then connect to the server to register itself. The server gives it a unique id which it will store in NV memory as well. Then it will transmit telemetry periodically to the server. In between transmissions it will switch off power to the peripherals using transistors and enter sleep mode to save as much power as possible. When the period timer runs out it boots up again and sends over telemetry. There is a physical pushbutton on the device as well for the user to interact with it. In sleep mode if it is pressed the device powers on and displays its information such as name, id, and last reading on the display. If the user presses the button again once it goes back to sleep. If they press it twice it will instantly send over telemetry. If they press it 3 times it will go back into setup mode and start the soft AP so you can reconfigure it. At any point in its operation other than sleep mode if the pushbutton is held for 5 seconds it will factory reset and clear all stored data so that it can be reconfigured.

Components

SH1106 I2C OLED - Displays information when necessary (Better user experience)
AHT10 I2C Temp/Humid Sensor - Collects telemetry data (Need it to actually measure data)
2x NPN 2N222 Transistors - Allows for shutting off power to peripherals
Pushbutton - Allows for user interaction
3x AAA Battery Pack - Provides 4.5 volts to the device


## Software & Libraries

This project is built using the [PlatformIO IDE](https://platformio.org/) and the Arduino framework.

*   **Arduino Framework:** The core framework providing basic functions for ESP32 development.
*   **`adafruit/Adafruit SH110X`:** Library for controlling the SH1106 I2C OLED display.
*   **`adafruit/Adafruit AHTX0`:** Library for interfacing with the AHT10 temperature and humidity sensor.
*   **`mathertel/OneButton`:** Simplifies handling of button presses (single, double, triple, long press) with debouncing.
*   **`bblanchon/ArduinoJson`:** Used for creating and parsing JSON payloads for API communication.
*   **`WebServer` & `DNSServer`:** Core Arduino libraries used by the `PortalManager` for creating the captive WiFi setup portal.
*   **`HTTPClient`:** Arduino library for making HTTP requests to the backend server.
*   **`Preferences`:** ESP32 library for storing configuration data persistently in Non-Volatile Storage (NVS).
*   **`WiFi`:** Arduino library for managing WiFi connectivity.
*   **`esp_sleep.h` (ESP-IDF):** Used by the `PowerManager` for configuring and entering ESP32's deep sleep mode.


Problems Faced

Originally I had planned to measure battery percentage to send as part of the telemetry. However as I was making the voltage divider (gives safe voltage to esp32 adc pin) I realized that it would constantly consume power and waste battery which completely defeats the purpose of sleep mode and shutting off peripherals. I decided to put a hold on that feature. 


Similar Embedded Devices

There are commercial products like this that do exist such as Monnit ALTA sensors, TempAlert, and Sensaphone devices but they are expensive, require subscriptions, or use proprietary gateways. The main advantage of my solution is that it is a lot cheaper, which makes it more affordable for small, family owned restaurants.


If I Had More Time

If I had more time for the project I would figure out a better way to implement the battery percentage readings because that is definitely an important feature of an IoT sensor. I would also add a case because right now it works on a breadboard. I would also add a switch (was planning to but couldn’t find one) so you can turn it on and off instead of having to remove the batteries to turn it off.


