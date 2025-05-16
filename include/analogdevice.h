#pragma once
#include "Objects.h"
#include <Arduino.h>

class TAnalogDevice;

void EmptyOnChangeState(TAnalogDevice *Device, unsigned int Value) {};

class TAnalogDevice : public TControl
{
private:
    unsigned int Value;
    uint32_t ReadTimeout{100};
    unsigned long LastTime{0};

public:
    uint8_t Pin{NULL};

    unsigned int GetState()
    {
        return Value;
    }

    void SetReadTimeout(uint32_t Timeout)
    {
        ReadTimeout = Timeout;
    }

    void ReadState()
    {
        unsigned int PinValue{0};
        PinValue = analogRead(Pin);

        if (PinValue != Value)
        {
            Value = PinValue;
            OnChageValue(this, Value);
        }
        LastTime = GetTimerValue();
    }

    TAnalogDevice(TObject *_Parent, uint8_t _Pin) : TControl(_Parent)
    {
        Pin = _Pin;
        pinMode(Pin, INPUT);
    };

    virtual void Idle()
    {
        unsigned long CurrentTime = GetTimerValue();

        if ((CurrentTime - LastTime) > ReadTimeout)
        {
            ReadState();
        }
    }

    void (*OnChageValue)(TAnalogDevice *Device, unsigned int Value){EmptyOnChangeState};

    ~TAnalogDevice();
};

TAnalogDevice::~TAnalogDevice()
{
}
