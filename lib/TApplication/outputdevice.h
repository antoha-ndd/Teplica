#pragma once
#include "simpledevice.h"

class TOutputDevice;


class TOutputDevice : public TSimpleDevice
{
private:


public:


    TOutputDevice( uint8_t _Pin) : TSimpleDevice(_Pin, TSimpleDeviceType::OutputDevice)
    {
        Prepare();
    };

    void On(){
        SetState(true);
    }

    void Off(){
        SetState(false);
    }


    void Toggle(){
        SetState(!GetState());
    }
    ~TOutputDevice();
};

inline TOutputDevice::~TOutputDevice()
{
}



