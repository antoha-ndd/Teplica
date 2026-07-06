#include <Arduino.h>

#include "config.h"
#include "var.h"
#include "settings.h"
#include "storage.h"
#include "MotorDriver.h"
#include "MotorQueueControl.h"
#include "mqtt.h"

// === Motor auto flags ===

void ApplyMotorAutoFlags()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (autoPaused[i])
            continue;

        MotorDriver[i]->AutoClose = data.ac[i];
        MotorDriver[i]->AutoOpen = data.ao[i];
    }
}

// === Auto-pause / timed restore ===

void PauseAutoControl(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    autoPaused[index] = true;
    MotorDriver[index]->AutoOpen = false;
    MotorDriver[index]->AutoClose = false;

    if (data.ar[index] > 0)
        autoRestoreAt[index] = millis() + (unsigned long)data.ar[index] * 60000UL;
    else
        autoRestoreAt[index] = 0;
}

void ApplyAutoFlagsFromWeb(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    data.ao[index] = MotorDriver[index]->AutoOpen;
    data.ac[index] = MotorDriver[index]->AutoClose;
    autoPaused[index] = false;
    autoRestoreAt[index] = 0;
    SaveSettings();
}

String GetAutoRestoreStatusText(int index)
{
    if (index < 0 || index >= MOTOR_COUNT || !autoPaused[index])
        return "-";

    if (autoRestoreAt[index] == 0)
        return "вручную";

    unsigned long now = millis();
    if (now >= autoRestoreAt[index])
        return "0";

    unsigned long msLeft = autoRestoreAt[index] - now;
    return String((msLeft + 59999UL) / 60000UL);
}

// === Automation (temperature) ===

void ProcessMotorAutomation(float temp)
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (autoPaused[i] || !data.motorEnabled[i])
            continue;

        if (data.ao[i] && !MotorDriver[i]->IsOpen() && temp > data.o[i])
        {
            TMotorQueueControl::RequestOpen(i);
            MqttPublishMotor(i);
        }

        if (data.ac[i] && MotorDriver[i]->IsOpen() && temp < data.c[i] - TEMP_HYSTERESIS)
        {
            TMotorQueueControl::RequestClose(i);
            MqttPublishMotor(i);
        }
    }
}
