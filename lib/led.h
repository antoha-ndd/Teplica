#pragma once
#include "outputdevice.h"

class TLed : public TOutputDevice
{
public:
    TLed(TObject *_Parent, uint8_t _Pin) : TOutputDevice(_Parent, _Pin){};
};