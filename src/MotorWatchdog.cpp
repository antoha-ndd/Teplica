#include <Arduino.h>

#include "config.h"
#include "var.h"
#include "MotorDriver.h"
#include "MotorWatchdog.h"
#include "MotorQueueControl.h"
#include "mqtt.h"

TMotorWatchdog::TMotorWatchdog() : TControl(nullptr), activeStartMs(0), lastActiveIndex(-1) {}
TMotorWatchdog::~TMotorWatchdog() {}

void TMotorWatchdog::Idle()
{
    int activeIndex = TMotorQueueControl::ActiveMotorIndex();

    if (activeIndex >= 0)
    {
        if (activeIndex != lastActiveIndex)
        {
            activeStartMs = millis();
            lastActiveIndex = activeIndex;
            return;
        }

        if (MotorDriver[activeIndex]->IsBusy() &&
            millis() - activeStartMs > 30000UL)
        {
            Serial.printf("[WARN] Motor %d watchdog timeout, stopping\n", activeIndex);
            TMotorQueueControl::Stop(activeIndex);
            MqttPublishMotor(activeIndex);
            lastActiveIndex = -1;
        }
    }
    else
    {
        lastActiveIndex = -1;
    }
}
