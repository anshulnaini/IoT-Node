# IoT Fleet - Telemetry Node

This project is for a single, low-power, ESP32-C3 based IoT node designed to be part of a larger IoT fleet. It collects environmental data (temperature and humidity) and reports it to a central web server. The node is designed for easy configuration and long-term operation on battery power.

---

## Features

*   **Easy WiFi & Endpoint Configuration:** On first boot or after a reset, the device hosts a WiFi Access Point and a captive portal to allow a user to configure WiFi credentials, server details, and device parameters from any phone or computer.
*   **Automatic Device Registration:** On its first connection, the node registers itself by sending its configured name and type to the server's `/api/devices` endpoint. The server responds with a unique Device ID (`deviceId`), which the node stores permanently for all future communication.
*   **Low-Power Operation:** Utilizes the ESP32's deep sleep mode to maximize battery life, waking only to send data at a user-defined interval. Peripherals are powered down via transistors during sleep.
*   **Data Telemetry:** Sends temperature, humidity, and battery percentage to the server's `/api/ingest` endpoint via HTTP POST.
*   **On-Demand Display:** An I2C OLED screen provides status information during setup and can be woken up on-demand to show current device status and data.
*   **Multi-Function Button:** A single pushbutton provides rich user interaction for viewing status, forcing a telemetry send, or re-entering setup mode.
*   **Persistent Configuration:** All settings are saved to Non-Volatile Storage, surviving power loss and reboots.

---

## Hardware Requirements

*   **Microcontroller:** ESP32-C3 (e.g., Seeed Studio XIAO ESP32C3, ESP32-C3-DevKitM-1)
*   **Display:** 128x64 I2C OLED Display (SH1106 or SSD1306)
*   **Sensor:** I2C Temperature & Humidity Sensor (e.g., SHT31, AHT20)
*   **Input:** 1x Tactile Pushbutton
*   **Power:**
    *   3x AAA Batteries
    *   2x NPN or N-Channel MOSFET transistors (for peripheral power switching)
    *   Resistors for button and transistor circuits.

---

## Software & Libraries

This project is built using the [PlatformIO IDE](https://platformio.org/).

*   **Framework:** Arduino
*   **Key Libraries:**
    *   `Adafruit_SH110X`: For the OLED display.
    *   `Adafruit SHT31 Library` (or similar): For the sensor.
    *   `WebServer` & `DNSServer`: For the captive portal.
    *   `HTTPClient`: For sending data to the server.
    *   `ArduinoJson`: For creating JSON payloads.
    *   `OneButton`: For easy handling of button presses.
    *   `Preferences`: For storing configuration in NV memory.

---

## How to Use

### First-Time Setup

1.  Power on the device. The OLED will indicate that it is in "Setup Mode".
2.  On your phone or computer, connect to the WiFi network named **"IoT-Node-Setup"**.
3.  A captive portal page should automatically open. If not, open a browser and navigate to `192.168.4.1`.
4.  Fill out the form with your home WiFi credentials, server details, and device preferences (name, type, etc.).
5.  Click "Submit". The device saves the configuration and connects to your WiFi.
6.  It will then send a `POST` request to `/api/devices` to register itself. The server will return a unique **Device ID**, which the node saves. The OLED will show the connection status.

### Normal Operation

*   The device will automatically wake from sleep at the specified interval, power on its sensors, collect data, send it to the `/api/ingest` endpoint, and go back to sleep.

### Button Controls

The pushbutton behavior depends on the device's current state:

| State                 | Press Type     | Action                                                              |
| --------------------- | -------------- | ------------------------------------------------------------------- |
| **Sleeping**          | Single Press   | Wake up and display device info (Name, UUID, etc.) for 30 seconds.    |
| **Info Display**      | Single Press   | Go back to sleep immediately.                                       |
| **Info Display**      | Double Press   | Trigger an immediate telemetry reading and send.                      |
| **Info Display**      | Triple Press   | Enter Setup Mode (launches the Soft AP).                            |
| **Any (except sleep)**| Hold (5 sec)   | **Factory Reset:** Clears all saved settings and reboots into Setup Mode. |

---

## API Interaction

### Device Registration (First Time Only)

The device performs a `POST` to `/api/devices` with the user-configured details.

**Request Body:**
```json
{
  "name": "Living Room Sensor",
  "type": "Temp/Humidity",
  "locationHint": "On the bookshelf"
}
```

The server responds with the full device object, from which the device extracts and saves the `id`.

**Server Response:**
```json
{
  "id": "clxja8xkq000008l5g1j2h3k4",
  "name": "Living Room Sensor",
  "type": "Temp/Humidity",
  "lastSeenAt": "2025-11-18T20:30:00.000Z",
  "tags": [],
  "locationHint": "On the bookshelf"
}
```

### Telemetry Ingest

For every measurement, the device performs a `POST` to `/api/ingest`. The server automatically handles the `timestamp`.

**Request Body:**
```json
{
  "deviceId": "clxja8xkq000008l5g1j2h3k4",
  "metrics": {
    "temperature_c": 23.4,
    "humidity_pct": 45.8,
    "battery_pct": 88.0
  },
  "extras": {
    "location": "Living Room"
  }
}
```
