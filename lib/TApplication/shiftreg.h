#pragma once
#include "Objects.h"
#include "ShiftRegister74HC595_NonTemplate.h"

class TShiftReg;


typedef void (*TShiftRegPinEvent)(TShiftReg *ShiftReg, size_t Pin, bool State);

inline void EmptyPinEvent(TShiftReg *ShiftReg, size_t Pin, bool State){}

class TShiftReg : public TControl, private ShiftRegister74HC595_NonTemplate
{
private:
    uint8_t EnablePin{0};
    size_t Size{0};
    bool *State{NULL};

public:
    TShiftRegPinEvent PinEvent{EmptyPinEvent};

    TShiftReg(TObject *_Parent,
              const uint8_t _size,
              const uint8_t serialDataPin,
              const uint8_t clockPin,
              const uint8_t latchPin,
              const uint8_t _EnablePin) : TControl(_Parent), ShiftRegister74HC595_NonTemplate(_size, serialDataPin, clockPin, latchPin)
    {
        EnablePin = _EnablePin;
        Size = _size*8;
        pinMode(EnablePin, OUTPUT);
        Disable();
        State = new bool[Size];
        for (size_t i = 0; i < Size; i++)
            State[i] = false;
    };

    void Enable()
    {
        digitalWrite(EnablePin, 0);
    }

    void Disable()
    {
        digitalWrite(EnablePin, 1);
    }

    void SetAllOff()
    {
        for (size_t i = 0; i < Size; i++)
            State[i] = false;
        setAllLow();        
        PinEvent(this,-1,false);
    }

    bool GetState(size_t Pin){
        if(Pin>=Size)
            return false;
        return State[Pin];

    }
    void SetAllOn()
    {
        for (size_t i = 0; i < Size; i++)
            State[i] = true;
        setAllHigh();        
        PinEvent(this,-1,true);
    }

    void SetPinState(size_t Pin, bool PinState){
        if(Pin>=Size)
            return;
        
        State[Pin] = PinState;
        set(Pin,PinState);
        PinEvent(this,Pin,State[Pin]);
    }

    void On(size_t Pin){
        SetPinState(Pin,true);
    }

    void Off(size_t Pin){
        SetPinState(Pin,false);
    }

    void Toggle(size_t Pin){
        if(Pin>=Size)
            return;
        SetPinState(Pin,!State[Pin]);

    }

    ~TShiftReg();
};

inline TShiftReg::~TShiftReg()
{
    delete[] State;
}

// ShiftRegister74HC595_NonTemplate *Shift;
