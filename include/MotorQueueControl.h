#pragma once

#include <stdint.h>
#include "Objects.h"

enum class MotorAction : uint8_t
{
    None,
    Open,
    Close,
    InitClose
};

struct MotorQueueEntry
{
    int8_t index;
    MotorAction action;
};

class TMotorQueueControl : public TControl
{
public:
    TMotorQueueControl();
    virtual ~TMotorQueueControl();

    virtual void Idle() override;

    static void RequestOpen(int index);
    static void RequestClose(int index);
    static void InitCloseAll();
    static void PauseForOta();
    static void ResumeAfterOta();

    static void ManualOpen(int index);
    static void ManualClose(int index);

    static bool IsOtaPaused() { return motorOtaPaused; }
    static int ActiveMotorIndex() { return activeMotorIndex; }
    static MotorAction ActiveMotorAction() { return activeMotorAction; }

    static void Stop(int index);

private:
    static bool IsCloseAction(MotorAction action);
    static bool IsSameDirection(MotorAction a, MotorAction b);
    static void ClearActiveMotor();
    static int FindQueueEntryForMotor(int index);
    static void RemoveQueueEntryForMotor(int index);
    static void PopQueueFront();
    static bool ExecuteMotorAction(int index, MotorAction action);
    static void EnqueueMotorAction(int index, MotorAction action);
    static void RequestMotorAction(int index, MotorAction action);

    static MotorQueueEntry motorQueue[3];
    static int motorQueueCount;
    static int8_t activeMotorIndex;
    static MotorAction activeMotorAction;
    static bool motorOtaPaused;
};
