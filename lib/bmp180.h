#pragma once
#include "Objects.h"
#include <Adafruit_BMP085.h>


class TBMP180 : public TControl, public Adafruit_BMP085
{
private:
    float TemperatureValue{0};  
    
public:
    
    TBMP180(TObject *_Parent) : TControl(_Parent)
    {
        begin();
    };

    void UpdateTemperature()
    {
        TemperatureValue = readTemperature();
    }

    float Temperature(bool Update = false){
        if(Update)
            UpdateTemperature();
        return TemperatureValue;
    }

    ~TBMP180();
};

TBMP180::~TBMP180()
{
}
