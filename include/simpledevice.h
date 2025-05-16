#pragma once
#include "Objects.h"

enum class TSimpleDeviceType : uint8_t
{
    InputDevicePullUp = INPUT_PULLUP,
    InputDevice = INPUT,
    OutputDevice = OUTPUT
};

class TSimpleDevice;

void EmptyOnChangeState(TSimpleDevice *Device, bool State) {};

class TSimpleDevice : public TControl
{
private:
    bool State{NULL};
    uint32_t ReadTimeout{50};
    unsigned long LastTime{0};

public:
    TSimpleDeviceType DeviceType{TSimpleDeviceType::OutputDevice};
    uint8_t Pin{NULL};

    void SetState(bool NewState)
    {

        State = NewState;
        WriteState();
        OnChageState(this, State);
    }

    bool GetState(){
        return State;
    }

    void SetReadTimeout(uint32_t Timeout){
        ReadTimeout=Timeout;
    }

    void WriteState()
    {

        digitalWrite(Pin, State);
        LastTime = GetTimerValue();
    }
    void ReadState()
    {

        bool PinState{false};

        if(DeviceType == TSimpleDeviceType::InputDevice)
            PinState = digitalRead(Pin);
        else if(DeviceType == TSimpleDeviceType::InputDevicePullUp)
            PinState = !digitalRead(Pin);
        
        if (PinState != State)
        {
            State = PinState;
            OnChageState(this, State);
        }
        LastTime = GetTimerValue();
    }

    TSimpleDevice(TObject *_Parent, uint8_t _Pin, TSimpleDeviceType _DeviceType) : TControl(_Parent)
    {

        Pin = _Pin;
        DeviceType = _DeviceType;

     
    };

    virtual void Idle()
    {

        if (DeviceType == TSimpleDeviceType::InputDevice || DeviceType == TSimpleDeviceType::InputDevicePullUp)
        {

            unsigned long CurrentTime = GetTimerValue();

            if ((CurrentTime - LastTime) > ReadTimeout)
                ReadState();
        }
    }

    void (*OnChageState)(TSimpleDevice *Device, bool State){EmptyOnChangeState};

    ~TSimpleDevice();
    void Prepare()
    {

        pinMode(Pin, static_cast<uint8_t>(DeviceType));
    }
};

TSimpleDevice::~TSimpleDevice()
{
}
