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

unsigned long GetTimerValueImpl();
extern TTimerValueCallback GetTimerValue;

extern SettingsGyver *ui;
extern Preferences preferences;

extern TApplication *App;
extern TWiFiControl *WiFiCtrl;
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
    float o[MOTOR_COUNT];
    float c[MOTOR_COUNT];
    bool ac[MOTOR_COUNT];
    bool ao[MOTOR_COUNT];
};

extern Data data;
