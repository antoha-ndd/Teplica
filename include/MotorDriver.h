#pragma once
#include "Objects.h"
#include "simpledevice.h"

class TMotorDriver;
void TMotorDriverEmptyOnChageState(TMotorDriver *Device){};

class TMotorDriver : public TControl
{
private:
    int OpenPin, ClosePin;
    TTimeStamp TimeStamp{0};
    bool State{false};

public:
    bool AutoOpen{false};
    bool AutoClose{false};
    TBMP180 *bmp{nullptr};
    float OpenValue{NAN};
    float CloseValue{NAN};

    TTimeStamp Timeout{10000};

    TSimpleDeviceType DeviceType{TSimpleDeviceType::OutputDevice};

    TMotorDriver(uint8_t _OpenPin, uint8_t _ClosePin) : TControl()
    {
        ClosePin = _ClosePin;
        OpenPin = _OpenPin;

        pinMode(OpenPin, OUTPUT);
        pinMode(ClosePin, OUTPUT);

        digitalWrite(OpenPin, LOW);
        digitalWrite(ClosePin, LOW);

    };

    void InitClose(){

        Open();
        Close();

    }

    void Open()
    {

        if(State) return;
        digitalWrite(OpenPin, LOW);
        digitalWrite(ClosePin, LOW);

        digitalWrite(OpenPin, HIGH);
        TimeStamp = millis();
        State = true;
        OnChageState(this);

    }

    bool IsOpen(){
        return State;
    }

    void Close()
    {
        if(!State) return;

        digitalWrite(OpenPin, LOW);
        digitalWrite(ClosePin, LOW);

        digitalWrite(ClosePin, HIGH);

        State = false;
        TimeStamp = millis();

        OnChageState(this);
    }

    void Idle(){


        if(!State && AutoOpen && !isnanf(OpenValue) && bmp!=nullptr){

            if(OpenValue > bmp->Temperature()) Open();

        }

        if(State && AutoClose && !isnanf(CloseValue) && bmp!=nullptr){

            if(CloseValue < bmp->Temperature()) Close();

        }

        if(TimeStamp == 0) return;

        TTimeStamp CurTime = millis();
        
        if( (CurTime - TimeStamp) > Timeout ){

            TimeStamp = 0;
            digitalWrite(OpenPin, LOW);
            digitalWrite(ClosePin, LOW);

        }

    }

    void (*OnChageState)(TMotorDriver *Device){TMotorDriverEmptyOnChageState};
};
