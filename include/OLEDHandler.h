#ifndef OLEDHANDLER_H
#define OLEDHANDLER_H

#include <Adafruit_SH110X.h>



class OLEDHandler {

public:

    // Constructor
    OLEDHandler(uint16_t SDA, uint16_t SCL);

    // function that initializes the OLED
    void initializeOLED();

    // function to display text
    void displayText(const char* text);

    /**
     * @brief Displays a formatted screen with key device information and sensor readings.
     * 
     * @param deviceName The configured name of the device.
     * @param deviceId The unique ID assigned by the server.
     * @param serverUrl The configured URL of the backend server.
     * @param temp The current temperature in Celsius.
     * @param humidity The current relative humidity.
     */
    void displayInfo(const char* deviceName, const char* deviceId, const char* serverUrl, float temp, float humidity);

    // function to clear the display
    void clearDisplay();


private:

    Adafruit_SH1106G display;

    //OLED Dimensions
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;

    // I2C pins
    u16_t sda_pin;
    u16_t scl_pin;

};


#endif 