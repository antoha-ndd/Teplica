#pragma once
#include "simpledevice.h"

class TOutputDevice;


class TOutputDevice : public TSimpleDevice
{
private:


public:


    TOutputDevice(TObject *_Parent, uint8_t _Pin) : TSimpleDevice(_Parent, _Pin, TSimpleDeviceType::OutputDevice)
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

TOutputDevice::~TOutputDevice()
{
}



