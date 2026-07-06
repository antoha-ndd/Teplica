#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "config.h"
#include "Objects.h"

class SettingsGyver;
class TApplication;
class TWiFiControl;
class TBMP180;
class TTimer;
class TSSD1306;
class TButton;
class TMotorDriver;
class TMQTTControl;

unsigned long GetTimerValueImpl();
extern TTimerValueCallback GetTimerValue;

extern SettingsGyver *ui;
extern Preferences preferences;

extern TApplication *App;
extern TWiFiControl *WiFiCtrl;
extern TMQTTControl *MQTTCtrl;
extern TBMP180 *bmp;
extern TTimer *Timer1;
extern TTimer *Timer2;
extern TSSD1306 *LCD;
extern TButton *BtnOpen[MOTOR_COUNT];
extern TButton *BtnClose[MOTOR_COUNT];
extern TMotorDriver *MotorDriver[MOTOR_COUNT];

struct Data
{
    char SSID[WIFI_FIELD_SIZE];
    char PWD[WIFI_FIELD_SIZE];
    char mqttHost[MQTT_FIELD_SIZE];
    char mqttTopic[MQTT_FIELD_SIZE];
    uint16_t mqttPort;
    float o[MOTOR_COUNT];
    float c[MOTOR_COUNT];
    bool ac[MOTOR_COUNT];
    bool ao[MOTOR_COUNT];
    uint16_t ar[MOTOR_COUNT];
};

extern Data data;
extern unsigned long autoRestoreAt[MOTOR_COUNT];
extern bool autoPaused[MOTOR_COUNT];

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

extern MotorQueueEntry motorQueue[MOTOR_COUNT];
extern int motorQueueCount;
extern int8_t activeMotorIndex;
extern MotorAction activeMotorAction;
