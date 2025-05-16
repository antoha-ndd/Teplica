#pragma once
#include "Objects.h"

class TAdvProcedure;

/// void EmptyOnChangeState(TSimpleDevice *Device, bool State) {};

typedef void (*TAdvProcedureCallback)(TAdvProcedure *Proc, void *Param);

void EmptyAdvProcedureCallback(TAdvProcedure *Proc, void *Param) {};

class TAdvProcedure : public TControl
{
private:
    unsigned long Timeout{0};
    bool Enabled{false};
    long long StartTime{0};

public:
    void *Param;
    TAdvProcedureCallback OnProcedure{NULL};
    TAdvProcedureCallback OnTimer{NULL};
    TAdvProcedureCallback OnStopTimer{NULL};
    TAdvProcedureCallback OnIdle{NULL};

    virtual void Idle()
    {

        unsigned long CurrentTime = millis();
        if (Enabled && (CurrentTime - StartTime) >= Timeout)
        {
            Enabled = false;
            if (OnTimer != NULL)
                OnTimer(this, Param);
        }

        if (OnIdle != NULL)
            OnIdle(this, Param);
    }

    void DisableTimer()
    {
        if (Enabled)
        {
            Enabled = false;
            if (OnStopTimer != NULL)
                OnStopTimer(this, Param);
        }
    };

    void Procedure(unsigned long _Timeout = 0)
    {
        if (_Timeout > 0)
        {
            Enabled = true;
            Timeout = _Timeout;
            StartTime = millis();
        }
        if (OnProcedure != NULL)
            OnProcedure(this, Param);
    };

    TAdvProcedure() : TControl(NULL){};
    ~TAdvProcedure(){};
};