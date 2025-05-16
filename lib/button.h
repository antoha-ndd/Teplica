#pragma once
#include "simpledevice.h"

class TButton;

void EmptyButtonEvent(TButton *Button) {};
void ButtonOnChangeState(TSimpleDevice *Device, bool State);

class TButton : public TSimpleDevice
{
private:


public:
    unsigned long FirsClick{0};
    int ClickCount{0};

    unsigned long DoubleClickTimeout{300};
    TButton(TObject *_Parent, uint8_t _Pin, bool PullUp = true) : TSimpleDevice(_Parent, _Pin, PullUp ? TSimpleDeviceType::InputDevicePullUp : TSimpleDeviceType::InputDevice)
    {
        OnChageState = ButtonOnChangeState;
        Prepare();
    };

    void (*OnPress)(TButton *Button){EmptyButtonEvent};
    void (*OnRelease)(TButton *Button){EmptyButtonEvent};
    void (*OnClick)(TButton *Button){EmptyButtonEvent};
    void (*OnDoubleClick)(TButton *Button){EmptyButtonEvent};

     void Idle();

    ~TButton();
};

void TButton::Idle()
{
    TSimpleDevice::Idle();
    unsigned long CurrentTime = GetTimerValue();
    if ((CurrentTime - FirsClick) > DoubleClickTimeout)
    {
        if(ClickCount==1)
            OnClick(this);
        else if(ClickCount==2)
            OnDoubleClick(this);

        FirsClick = 0;
        ClickCount = 0;
    }
}

TButton::~TButton()
{
}

void ButtonOnChangeState(TSimpleDevice *Device, bool State)
{
    TButton *Button = static_cast<TButton *>(Device);

    if (State == false)
    {
        Button->OnRelease(Button);

        unsigned long CurrentTime = GetTimerValue();

        if (Button->ClickCount == 0)
            Button->FirsClick = CurrentTime;

        Button->ClickCount++;
    }
    else
        Button->OnPress(Button);
};