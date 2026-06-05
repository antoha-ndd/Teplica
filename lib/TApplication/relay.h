#pragma once
#include "outputdevice.h"

class TRelay : public TOutputDevice
{
public:
    TRelay(TApplication *App, uint8_t _Pin) : TOutputDevice(_Pin)
    {
        Register(App);
    }
};