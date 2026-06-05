#pragma once
#include "outputdevice.h"

class TLed : public TOutputDevice
{
public:
    TLed( uint8_t _Pin) : TOutputDevice(_Pin){};
};