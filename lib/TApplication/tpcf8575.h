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

inline void EmptyTPCF8575ButtonEvent(TPCF8575_Button *Button) {};

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
        PCF->begin();
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
    bool State{false};

public:


    TPCF8575_OutputDevice(TPCF8575 *_PCF, uint8_t _Pin) : TControl(NULL)
    {
        Pin = _Pin;
        PCF = _PCF;
        PCF->pinMode(Pin, OUTPUT);
        PCF->begin();
        PCF->digitalWrite( Pin , LOW );

    };

    bool GetState(){
        return State;
    }

    void SetState( bool _State){

        State = _State;
        if(_State) PCF->digitalWrite( Pin , HIGH );
        else PCF->digitalWrite( Pin , LOW );
//        Serial.print("SET STATE : ");
  //      Serial.println(State);

    }

    void Toggle(){

        State = !State;
        SetState( State );

    }

    int GetPin(){
        return Pin;
    }


};


