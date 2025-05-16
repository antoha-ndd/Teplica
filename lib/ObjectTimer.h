#pragma once
#include "Objects.h"



class TTimer;

void EmptyTimer(TTimer *Timer){};

class TTimer:public TControl
{
private:
    
    bool Enabled{false};
    bool OnWork{false};
    unsigned long long int LastTime{0};

public:
    unsigned long int Timeout{0};

    TTimer(TObject *_Parent):TControl(_Parent){

        OnTimeout = EmptyTimer;
    }

    TTimer():TControl(){
        OnTimeout = EmptyTimer;
    }


    void Start(unsigned long int _Timeout){

        Timeout = _Timeout;
        Enabled = true;

    }

    void Stop(){

        Enabled = false;

    }

    void Idle(){
        
        if(!Enabled || OnWork) return;
        
        unsigned long int CurrentTime = GetTimerValue();

        if((CurrentTime - LastTime)>=Timeout){
            
            LastTime    = CurrentTime;
            OnWork      = true;
            
            OnTimeout(this);
            
            OnWork      = false;
            
        }

    }    

    void (*OnTimeout)(TTimer *Timer);

    ~TTimer();
};


TTimer::~TTimer()
{
}
