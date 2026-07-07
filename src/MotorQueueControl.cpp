#include <Arduino.h>

#include "config.h"
#include "var.h"
#include "MotorDriver.h"
#include "mqtt.h"
#include "settings.h"
#include "MotorQueueControl.h"

// === Static state ===

MotorQueueEntry TMotorQueueControl::motorQueue[MOTOR_COUNT];
int TMotorQueueControl::motorQueueCount = 0;
int8_t TMotorQueueControl::activeMotorIndex = -1;
MotorAction TMotorQueueControl::activeMotorAction = MotorAction::None;
bool TMotorQueueControl::motorOtaPaused = false;

// === Construction ===

TMotorQueueControl::TMotorQueueControl() : TControl(nullptr) {}
TMotorQueueControl::~TMotorQueueControl() {}

void TMotorQueueControl::Idle()
{
    // === Motor queue processing ===
    if (!motorOtaPaused)
    {
        if (activeMotorIndex >= 0)
        {
            if (!MotorDriver[activeMotorIndex]->IsBusy())
                ClearActiveMotor();
        }

        // Process queued actions only when no motor is active
        if (activeMotorIndex < 0)
        {
            while (motorQueueCount > 0)
            {
                const int index = motorQueue[0].index;
                const MotorAction action = motorQueue[0].action;
                PopQueueFront();

                if (ExecuteMotorAction(index, action))
                    break;
            }
        }
    }

    // === Auto-restore timers ===
    unsigned long now = millis();
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (autoRestoreAt[i] == 0 || now < autoRestoreAt[i])
            continue;

        autoRestoreAt[i] = 0;
        autoPaused[i] = false;
        MotorDriver[i]->AutoOpen = data.ao[i];
        MotorDriver[i]->AutoClose = data.ac[i];
        MqttPublishMotor(i);
    }
}

// === Queue helpers ===

bool TMotorQueueControl::IsCloseAction(MotorAction action)
{
    return action == MotorAction::Close || action == MotorAction::InitClose;
}

bool TMotorQueueControl::IsSameDirection(MotorAction a, MotorAction b)
{
    if (a == b)
        return true;
    return IsCloseAction(a) && IsCloseAction(b);
}

void TMotorQueueControl::ClearActiveMotor()
{
    activeMotorIndex = -1;
    activeMotorAction = MotorAction::None;
}

int TMotorQueueControl::FindQueueEntryForMotor(int index)
{
    for (int i = 0; i < motorQueueCount; i++)
    {
        if (motorQueue[i].index == index)
            return i;
    }
    return -1;
}

void TMotorQueueControl::RemoveQueueEntryForMotor(int index)
{
    for (int i = 0; i < motorQueueCount;)
    {
        if (motorQueue[i].index == index)
        {
            for (int j = i; j < motorQueueCount - 1; j++)
                motorQueue[j] = motorQueue[j + 1];
            motorQueueCount--;
        }
        else
        {
            i++;
        }
    }
}

void TMotorQueueControl::PopQueueFront()
{
    if (motorQueueCount <= 0)
        return;

    for (int i = 0; i < motorQueueCount - 1; i++)
        motorQueue[i] = motorQueue[i + 1];
    motorQueueCount--;
}

bool TMotorQueueControl::ExecuteMotorAction(int index, MotorAction action)
{
    if (index < 0 || index >= MOTOR_COUNT || action == MotorAction::None)
        return false;

    activeMotorIndex = index;
    activeMotorAction = action;

    if (action == MotorAction::Open)
    {
        if (MotorDriver[index]->IsOpen())
        {
            ClearActiveMotor();
            return false;
        }
        MotorDriver[index]->Open();
    }
    else if (action == MotorAction::InitClose)
    {
        MotorDriver[index]->InitClose();
    }
    else
    {
        if (!MotorDriver[index]->IsOpen())
        {
            ClearActiveMotor();
            return false;
        }
        MotorDriver[index]->Close();
    }

    if (!MotorDriver[index]->IsBusy())
    {
        ClearActiveMotor();
        return false;
    }

    MqttPublishMotor(index);
    return true;
}

void TMotorQueueControl::EnqueueMotorAction(int index, MotorAction action)
{
    if (motorQueueCount >= MOTOR_COUNT)
        return;

    motorQueue[motorQueueCount].index = (int8_t)index;
    motorQueue[motorQueueCount].action = action;
    motorQueueCount++;
}

void TMotorQueueControl::RequestMotorAction(int index, MotorAction action)
{
    if (motorOtaPaused)
        return;

    if (index < 0 || index >= MOTOR_COUNT || action == MotorAction::None)
        return;

    if (!data.motorEnabled[index])
        return;

    if (activeMotorIndex == index)
    {
        if (IsSameDirection(activeMotorAction, action))
            return;

        MotorDriver[index]->Stop();
        ClearActiveMotor();
        ExecuteMotorAction(index, action);
        return;
    }

    const int queueIndex = FindQueueEntryForMotor(index);
    if (queueIndex >= 0)
    {
        if (IsSameDirection(motorQueue[queueIndex].action, action))
            return;

        RemoveQueueEntryForMotor(index);
    }

    if (activeMotorIndex < 0)
    {
        ExecuteMotorAction(index, action);
        return;
    }

    EnqueueMotorAction(index, action);
}

// === Public API ===

void TMotorQueueControl::RequestOpen(int index)
{
    RequestMotorAction(index, MotorAction::Open);
}

void TMotorQueueControl::RequestClose(int index)
{
    RequestMotorAction(index, MotorAction::Close);
}

void TMotorQueueControl::InitCloseAll()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
        RequestMotorAction(i, MotorAction::InitClose);
}

void TMotorQueueControl::PauseForOta()
{
    motorOtaPaused = true;

    if (activeMotorIndex >= 0)
    {
        MotorDriver[activeMotorIndex]->Stop();
        ClearActiveMotor();
    }

    motorQueueCount = 0;
}

void TMotorQueueControl::ResumeAfterOta()
{
    motorOtaPaused = false;
}

void TMotorQueueControl::ManualOpen(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    RequestOpen(index);
    PauseAutoControl(index);
    MqttPublishMotor(index);
}

void TMotorQueueControl::ManualClose(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    RequestClose(index);
    PauseAutoControl(index);
    MqttPublishMotor(index);
}

void TMotorQueueControl::Stop(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    if (activeMotorIndex == index)
    {
        MotorDriver[index]->Stop();
        ClearActiveMotor();
    }

    RemoveQueueEntryForMotor(index);
    MqttPublishMotor(index);
}
