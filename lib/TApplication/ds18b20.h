#pragma once
#include "Objects.h"

#include <OneWire.h>
#include <DallasTemperature.h>

class TSensor_DS18B20
{
private:
    uint8_t Pin;
    OneWire *OneWireBus{NULL};
    DallasTemperature *Sensor{NULL};
    bool Initialized{false};
    float TemperatureValue{0};

    void Initialize()
    {
        if (Initialized)
            return;

        OneWireBus = new OneWire(Pin);
        Sensor = new DallasTemperature(OneWireBus);
        Sensor->begin();
        Initialized = true;
    }

public:
    
    TSensor_DS18B20(uint8_t _Pin)
    {
        Pin = _Pin;
    };
    void UpdateTemperature()
    {
        Initialize();
        Sensor->requestTemperatures();
        TemperatureValue = Sensor->getTempCByIndex(0);
    }

    float Temperature(bool Update = false){
        if(Update)
            UpdateTemperature();
        return TemperatureValue;
    }

    ~TSensor_DS18B20();
};

inline TSensor_DS18B20::~TSensor_DS18B20()
{
    if (Sensor != NULL)
        delete Sensor;

    if (OneWireBus != NULL)
        delete OneWireBus;
}
