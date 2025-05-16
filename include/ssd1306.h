#pragma once
#include "Objects.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

class TSSD1306: public TControl,public Adafruit_SSD1306{    
public:

     
    TSSD1306() : TControl(NULL),Adafruit_SSD1306()
    {

        begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
        clearDisplay();
        
        setTextSize(1); // установка размера шрифта
        setTextColor(WHITE); // установка цвета текста
        setCursor(0, 0); // установка курсора
        display();
    };

    ~TSSD1306();
};

TSSD1306::~TSSD1306()
{
}
