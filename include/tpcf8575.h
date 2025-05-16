#pragma once
#include "simpledevice.h"
#include "PCF8575.h"

class TPCF8575 : public PCF8575
{
public:
    TPCF8575(uint8_t deviceAddress = 0x20) : PCF8575(deviceAddress)
    {

        begin();
    };
};

class TPCF8575_Button;

void EmptyTPCF8575ButtonEvent(TPCF8575_Button *Button) {};
void TPCF8575_ButtonOnChangeState(TSimpleDevice *Device, bool State);

class TPCF8575_Button : public TControl
{
private:
    TPCF8575 *PCF{NULL};
    uint8_t Pin{0};
    uint8_t State{0};
    uint32_t ReadTimeout{50};
    unsigned long LastTime{0};

    void ChageState()
    {

        if (State == false)
        {
            OnRelease(this);
            OnClick(this);
        }
        else
            OnPress(this);
    }

    void ReadState()
    {
        uint8_t PinState{0};

        PinState = PCF->digitalRead(Pin);

        if (PinState != State)
        {
            State = PinState;
            ChageState();
        }

        LastTime = GetTimerValue();
    }

public:


    TPCF8575_Button(TPCF8575 *_PCF, uint8_t _Pin) : TControl(NULL)
    {
        Pin = _Pin;
        PCF = _PCF;
        PCF->pinMode(Pin, INPUT);
        Serial.println("CREATE");
    };

    void (*OnPress)(TPCF8575_Button *Button){EmptyTPCF8575ButtonEvent};
    void (*OnRelease)(TPCF8575_Button *Button){EmptyTPCF8575ButtonEvent};
    void (*OnClick)(TPCF8575_Button *Button){EmptyTPCF8575ButtonEvent};
    
    int GetPin(){
        return Pin;
    }

    void Idle()
    {
        unsigned long CurrentTime = GetTimerValue();
    
        if ((CurrentTime - LastTime) > ReadTimeout)
            ReadState();
    }
};


class TPCF8575_OutputDevice : public TControl
{
private:
    TPCF8575 *PCF{NULL};
    uint8_t Pin{0};
    uint8_t State{0};

public:


    TPCF8575_OutputDevice(TPCF8575 *_PCF, uint8_t _Pin) : TControl(NULL)
    {
        Pin = _Pin;
        PCF = _PCF;
        PCF->pinMode(Pin, OUTPUT);
        
    };

    uint8_t GetState(){
        return State;
    }

    void SetState( uint8_t _State){

        State = _State;
        PCF->digitalWrite( Pin , State );

    }

    int GetPin(){
        return Pin;
    }


};


