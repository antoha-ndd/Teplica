#pragma once
#include "simpledevice.h"

#include "PCF8574.h"


class TPCF8574 : public PCF8574
{
public:
    TPCF8574(uint8_t deviceAddress = 0x20) : PCF8574(deviceAddress)
    {
        setAddress(deviceAddress);
        begin();
    };
};


class TPCF8574_Button;

void EmptyTPCF8574ButtonEvent(TPCF8574_Button *Button) {};
void TPCF8574_ButtonOnChangeState(TSimpleDevice *Device, bool State);

class TPCF8574_Button : public TControl
{
private:
    TPCF8574 *PCF{NULL};
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

        PinState = PCF->read(Pin);

        if (PinState != State)
        {
            State = PinState;
            ChageState();
        }

        LastTime = GetTimerValue();
    }

public:


    TPCF8574_Button(TPCF8574 *_PCF, uint8_t _Pin) : TControl(NULL)
    {
        Pin = _Pin;
        PCF = _PCF;
        PCF->begin();
    };

    void (*OnPress)(TPCF8574_Button *Button){EmptyTPCF8574ButtonEvent};
    void (*OnRelease)(TPCF8574_Button *Button){EmptyTPCF8574ButtonEvent};
    void (*OnClick)(TPCF8574_Button *Button){EmptyTPCF8574ButtonEvent};
    
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


class TPCF8574_OutputDevice : public TControl
{
private:
    TPCF8574 *PCF{NULL};
    uint8_t Pin{0};
    bool State{false};

public:


    TPCF8574_OutputDevice(TPCF8574 *_PCF, uint8_t _Pin) : TControl(NULL)
    {
        Pin = _Pin;
        PCF = _PCF;
        PCF->begin();
        PCF->write( Pin , LOW );

    };

    bool GetState(){
        return State;
    }

    void SetState( bool _State){

        State = _State;
        if(_State) PCF->write( Pin , HIGH );
        else PCF->write( Pin , LOW );
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


