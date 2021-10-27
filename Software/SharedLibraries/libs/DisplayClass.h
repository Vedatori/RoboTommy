#ifndef _DISPLAY_CLASS_
#define _DISPLAY_CLASS_

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Strings_data.h"

class DisplayClass{
    Adafruit_SSD1306 display;
public:
    void init(){
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
    }
    void writeWelcome(){
        display.clearDisplay();
        display.drawBitmap(0, 0, startDisplay, 128, 32, 1);
        display.display();
    }
    void writeString(const String aText){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.print(aText);
        display.display();
    }
    void writeString(const String aText, const String bText){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.print(aText);
        display.setCursor(0, 20);
        display.print(bText);
        display.display();
    }
};
#endif /*_DISPLAY_CLASS_*/