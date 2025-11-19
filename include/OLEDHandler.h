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