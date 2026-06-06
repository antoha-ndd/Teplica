#pragma once
#include "Objects.h"
#include <Adafruit_BMP085.h>


class TBMP180 : public TControl, public Adafruit_BMP085
{
private:
    float TemperatureValue{0};
    bool sensorOk{false};

public:
    TBMP180(TObject *_Parent) : TControl(_Parent)
    {
        sensorOk = begin();
    }

    bool IsOk() const
    {
        return sensorOk;
    }

    void UpdateTemperature()
    {
        if (!sensorOk)
            return;

        TemperatureValue = readTemperature();
    }

    float Temperature(bool Update = false)
    {
        if (Update)
            UpdateTemperature();
        return TemperatureValue;
    }

    ~TBMP180();
};

inline TBMP180::~TBMP180()
{
}
