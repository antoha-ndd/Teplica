#pragma once

#include "Objects.h"

class TMotorWatchdog : public TControl
{
public:
    TMotorWatchdog();
    virtual ~TMotorWatchdog();

    virtual void Idle() override;

private:
    unsigned long activeStartMs;
    int8_t lastActiveIndex;
};
