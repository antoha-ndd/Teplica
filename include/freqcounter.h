#pragma once
#include "simpledevice.h"

class TFreqCounter;

void FreqOnChangeState(TSimpleDevice *Device, bool State);

void EmptyOnChangeFreq(TFreqCounter *Device, unsigned int Freq) {};

class TFreqCounter : public TSimpleDevice
{
private:
public:
    int Counter{0};
    int Freq{0};
    int Sens{0};
    TFreqCounter(TObject *_Parent, uint8_t _Pin, bool PullUp = true) : TSimpleDevice(_Parent, _Pin, PullUp ? TSimpleDeviceType::InputDevicePullUp : TSimpleDeviceType::InputDevice)
    {
        OnChageState = FreqOnChangeState;
        Prepare();
        SetReadTimeout(5);
    };

    void Idle();
    void (*OnChangeFreq)(TFreqCounter *Device, unsigned int Counter){EmptyOnChangeFreq};

    ~TFreqCounter();
};

void TFreqCounter::Idle()
{
    TSimpleDevice::Idle();
    static int F1{0}, F2{0}, F3{0}, NoEqual{0};

    static unsigned long Tick{0};
    unsigned long CurrentTime = GetTimerValue();

    if ((CurrentTime - Tick) >= 1000)
    {
        int DeltaF = Counter - Freq;
        int MidF{0};
        F1 = F2;
        F2 = F3;
        F3 = Counter;
        MidF = (F1 + F2 + F3) / 3;

        if (MidF != Freq)
            NoEqual++;
        else
            NoEqual = 0;

        // Serial.println("COUNTER : " + String(Counter) + " FREQ : " + String(Freq) + " DELTA : " + String(DeltaF) + " MID : " + String(MidF) + " NOEQ : " + String(NoEqual));
        if (DeltaF > Sens || DeltaF < -Sens || NoEqual > 3)
        {
            //    Serial.println("CHANGE");
            Freq = Counter;
            NoEqual = 0;
            OnChangeFreq(this, Freq);
        }

        Counter = 0;
        Tick = CurrentTime;
    }
}

TFreqCounter::~TFreqCounter()
{
}

void FreqOnChangeState(TSimpleDevice *Device, bool State)
{
    TFreqCounter *Freq = static_cast<TFreqCounter *>(Device);

    if (State)
        Freq->Counter++;
}