#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "var.h"
#include "events.h"
#include "settings.h"
#include "mqtt.h"
#include "bmp180.h"
#include "button.h"
#include "MotorDriver.h"
#include "ssd1306.h"
#include "ObjectTimer.h"
#include "app/WiFiControl.h"

static void ShowMotorLabel(const char *action, int index)
{
    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print(String(action) + " " + String(index + 1));
    LCD->display();
}

static void BtnOpen_Click(int index, TButton *Button)
{
    (void)Button;
    ShowMotorLabel("Open", index);
    ManualOpen(index);
}

static void BtnClose_Click(int index, TButton *Button)
{
    (void)Button;
    ShowMotorLabel("Close", index);
    ManualClose(index);
}

void OnOTAProgress(unsigned int Progress, unsigned int Total)
{
    if (!LCD)
        return;

    static int Pr = 0;
    int CurrentPr = (Progress * 100) / Total;

    if (CurrentPr != Pr)
    {
        Pr = CurrentPr;
        LCD->clearDisplay();
        LCD->setCursor(10, 1);
        LCD->setTextSize(1);
        LCD->print("Updating firmware");
        LCD->setCursor(45, 15);
        LCD->setTextSize(2);
        LCD->print(String(Pr) + "%");
        LCD->display();
    }
}

void BtnOpen0_Click(TButton *Button) { BtnOpen_Click(0, Button); }
void BtnOpen1_Click(TButton *Button) { BtnOpen_Click(1, Button); }
void BtnOpen2_Click(TButton *Button) { BtnOpen_Click(2, Button); }

void BtnClose0_Click(TButton *Button) { BtnClose_Click(0, Button); }
void BtnClose1_Click(TButton *Button) { BtnClose_Click(1, Button); }
void BtnClose2_Click(TButton *Button) { BtnClose_Click(2, Button); }

void BtnPairing_DblClick(TButton *Button)
{
    (void)Button;
    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(1);
    LCD->print("Pairing...");
    LCD->display();
    Pairing();
}

void Timer1_Timeout(TTimer *Timer)
{
    (void)Timer;
    static bool NetInfo{false};
    Serial.println(String(bmp->Temperature(true)));

    if (NetInfo)
    {
        LCD->clearDisplay();
        LCD->setCursor(1, 1);
        LCD->setTextSize(1);
        LCD->println(String("SSID : " + WiFi.SSID()).c_str());
        LCD->println(String("RSSI : " + String(WiFi.RSSI())).c_str());
        LCD->println(String("IP : " + WiFi.localIP().toString()).c_str());
        LCD->println(String("Connected : " + String(WiFi.isConnected())).c_str());
        LCD->display();
    }
    else
    {
        LCD->clearDisplay();
        LCD->setCursor(20, 5);
        LCD->setTextSize(3);
        LCD->print(String(bmp->Temperature()).c_str());
        LCD->display();
    }
    NetInfo = !NetInfo;
}

void Timer2_Timeout(TTimer *Timer)
{
    (void)Timer;

    TickAutoRestore();

    float temp = bmp->Temperature(true);
    ProcessMotorAutomation(temp);

    MqttPublishTelemetry();
    MqttPublishAllMotors();
}
