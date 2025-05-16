#pragma once
#include "outputdevice.h"

class TRelay : public TOutputDevice
{
public:
    TRelay(TObject *_Parent, uint8_t _Pin) : TOutputDevice(_Parent, _Pin){};
};