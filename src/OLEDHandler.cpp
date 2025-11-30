#include <OLEDHandler.h>
#include <Wire.h>   


const int OLEDHandler::SCREEN_WIDTH = 128;
const int OLEDHandler::SCREEN_HEIGHT = 64;


// Constructor
OLEDHandler::OLEDHandler(uint16_t SDA, uint16_t SCL)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire) {   // Intitalize a display instance
    sda_pin = SDA;
    scl_pin = SCL;
    }



//Function to initialize the OLED
void OLEDHandler::initializeOLED() {

    //Start I2C communication
    Wire.begin(sda_pin, scl_pin);

    //Attempt communication with OLED
    if (!display.begin(0x3C)){
        Serial.println("Failed to connect to OLED");
    }

    display.clearDisplay();     // Clear display buffer
    display.display();          // Send out buffer
}


//Function to display some text at center of OLED
void OLEDHandler::displayText(const char* text) {
    int16_t x1, y1;
    uint16_t w, h;
    display.clearDisplay();
    display.setTextSize(1);

    //Calc center
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (display.width() - w) / 2; 
    int16_t y = (display.height() - h) / 2;

    display.setCursor(x,y);
    display.setTextColor(SH110X_WHITE);
    display.println(text);
    display.display();
}

void OLEDHandler::displayInfo(const char* deviceName, const char* deviceId, const char* serverUrl, float temp, float humidity) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);

    display.print("Name: ");
    display.println(deviceName);

    display.print("ID: ");
    char id_short[13];
    strncpy(id_short, deviceId, 12);
    id_short[12] = '\0';
    display.println(id_short);

    display.print("Server: ");
    // shorten serverUrl if too long
    char url_short[31]; 
    if (strlen(serverUrl) > 30) {
        strncpy(url_short, serverUrl, 27); 
        strcpy(&url_short[27], "...");
        url_short[30] = '\0';
        display.println(url_short);
    } else {
        display.println(serverUrl);
    }

    display.setCursor(0, 40);
    display.print("Temp: ");
    display.print(temp, 1);
    display.println(" C");

    display.setCursor(0, 50);
    display.print("Humi: ");
    display.print(humidity, 1);
    display.println(" %");

    display.display();
}


// Fuction to clear the display


void OLEDHandler::clearDisplay() {
    display.clearDisplay();
    display.display();
}


