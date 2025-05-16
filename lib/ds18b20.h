#pragma once
#include "Objects.h"

#include <OneWire.h>
#include <DallasTemperature.h>

class TSensor_DS18B20 : public TControl, public DallasTemperature
{
private:
    uint8_t Pin;
    OneWire oneWire;
    float TemperatureValue{0};  
public:
    
    TSensor_DS18B20(TObject *_Parent, uint8_t _Pin) : TControl(_Parent)
    {
        Pin = _Pin;
        oneWire.begin(Pin);
        setOneWire(&oneWire);
        begin();
    };
    void UpdateTemperature()
    {
        requestTemperatures();
        TemperatureValue = getTempCByIndex(0);
    }

    float Temperature(bool Update = false){
        if(Update)
            UpdateTemperature();
        return TemperatureValue;
    }

    ~TSensor_DS18B20();
};

TSensor_DS18B20::~TSensor_DS18B20()
{
}
