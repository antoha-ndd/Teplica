#include <Arduino.h>

#include "hardware_pins.h"
#include "var.h"
#include "events.h"
#include "settings.h"
#include "webui.h"
#include "AppSettings.h"
#include "button.h"
#include "MotorDriver.h"
#include "ObjectTimer.h"
#include "bmp180.h"
#include "ssd1306.h"
#include "app/WiFiControl.h"
#include "mqtt.h"

void Init()
{
    Serial.begin(57600);

    App = new TApplication();
    App->Run();

    WiFiCtrl = new TWiFiControl();
    WiFiCtrl->Register(App);

    LCD = new TSSD1306();
    ArduinoOTA.onProgress(OnOTAProgress);
    ArduinoOTA.onStart([]() { PauseMotorsForOta(); });
    ArduinoOTA.onEnd([]() { ResumeMotorsAfterOta(); });

    LCD->clearDisplay();
    LCD->setCursor(20, 5);
    LCD->setTextSize(3);
    LCD->print("START");
    LCD->display();
    delay(1000);

    bmp = new TBMP180(NULL);
    bmp->Register(App);

    Timer1 = new TTimer();
    Timer1->OnTimeout = Timer1_Timeout;
    Timer1->Register(App);

    Timer2 = new TTimer();
    Timer2->OnTimeout = Timer2_Timeout;
    Timer2->Register(App);

    BtnOpen[0] = new TButton(PIN_BTN_OPEN_0, false);
    BtnOpen[0]->OnClick = BtnOpen0_Click;
    BtnOpen[0]->OnDoubleClick = BtnPairing_DblClick;
    BtnOpen[0]->Register(App);

    BtnOpen[1] = new TButton(PIN_BTN_OPEN_1, false);
    BtnOpen[1]->OnClick = BtnOpen1_Click;
    BtnOpen[1]->Register(App);

    BtnOpen[2] = new TButton(PIN_BTN_OPEN_2, false);
    BtnOpen[2]->OnClick = BtnOpen2_Click;
    BtnOpen[2]->Register(App);

    BtnClose[0] = new TButton(PIN_BTN_CLOSE_0, false);
    BtnClose[0]->OnClick = BtnClose0_Click;
    BtnClose[0]->Register(App);

    BtnClose[1] = new TButton(PIN_BTN_CLOSE_1, false);
    BtnClose[1]->OnClick = BtnClose1_Click;
    BtnClose[1]->Register(App);

    BtnClose[2] = new TButton(PIN_BTN_CLOSE_2, false);
    BtnClose[2]->OnClick = BtnClose2_Click;
    BtnClose[2]->Register(App);

    MotorDriver[0] = new TMotorDriver(PIN_MOTOR0_OPEN, PIN_MOTOR0_CLOSE);
    MotorDriver[1] = new TMotorDriver(PIN_MOTOR1_OPEN, PIN_MOTOR1_CLOSE);
    MotorDriver[2] = new TMotorDriver(PIN_MOTOR2_OPEN, PIN_MOTOR2_CLOSE);

    MotorDriver[0]->Register(App);
    MotorDriver[1]->Register(App);
    MotorDriver[2]->Register(App);

    InitCloseAllMotors();

    LoadSettings();
    InitMqtt();
    InitWebUi();

    Timer1->Start(5000);
    Timer2->Start(1000);
}
