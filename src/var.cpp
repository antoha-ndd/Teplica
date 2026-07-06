#include "bmp180.h"
#include "button.h"
#include "MotorDriver.h"
#include "ObjectTimer.h"
#include "ssd1306.h"
#include "app/WiFiControl.h"
#include "var.h"

unsigned long GetTimerValueImpl()
{
    return millis();
}

TTimerValueCallback GetTimerValue = GetTimerValueImpl;

Preferences preferences;

TApplication *App;
TWiFiControl *WiFiCtrl;
TMQTTControl *MQTTCtrl;
TBMP180 *bmp;
TTimer *Timer1;
TTimer *Timer2;
TSSD1306 *LCD;
TButton *BtnOpen[MOTOR_COUNT];
TButton *BtnClose[MOTOR_COUNT];
TMotorDriver *MotorDriver[MOTOR_COUNT];

Data data;
unsigned long autoRestoreAt[MOTOR_COUNT]{0};
bool autoPaused[MOTOR_COUNT]{false};
