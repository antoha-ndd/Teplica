#include <Arduino.h>
#include <cstring>

#include "config.h"
#include "var.h"
#include "settings.h"
#include "MotorDriver.h"
#include "app/WiFiControl.h"
#include "mqtt.h"

void NormalizeThresholds()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (data.o[i] <= data.c[i])
            data.o[i] = data.c[i] + TEMP_HYSTERESIS * 2.0f;
    }
}

void CopyStringField(char *dest, size_t size, const String &src)
{
    if (size == 0)
        return;

    strncpy(dest, src.c_str(), size - 1);
    dest[size - 1] = '\0';
}

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

void LoadSettings()
{
    preferences.begin("config", true);

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        String closeKey = "c" + String(i + 1);
        String openKey = "o" + String(i + 1);
        String acKey = "ac" + String(i + 1);
        String aoKey = "ao" + String(i + 1);
        String arKey = "ar" + String(i + 1);

        if (preferences.isKey(closeKey.c_str()))
            data.c[i] = preferences.getFloat(closeKey.c_str(), DEFAULT_TEMP_CLOSE);
        else
            data.c[i] = DEFAULT_TEMP_CLOSE;

        if (preferences.isKey(openKey.c_str()))
            data.o[i] = preferences.getFloat(openKey.c_str(), DEFAULT_TEMP_OPEN);
        else
            data.o[i] = DEFAULT_TEMP_OPEN;
        data.ac[i] = preferences.getBool(acKey.c_str(), false);
        data.ao[i] = preferences.getBool(aoKey.c_str(), false);
        data.ar[i] = (uint16_t)preferences.getUInt(arKey.c_str(), 0);
    }

    String ssid = preferences.getString("wifi_ssid", "");
    if (ssid.length() == 0)
        ssid = preferences.getString("SSID", "");

    String pwd = preferences.getString("wifi_pass", "");
    if (pwd.length() == 0)
        pwd = preferences.getString("PWD", "");

    CopyStringField(data.SSID, WIFI_FIELD_SIZE, ssid);
    CopyStringField(data.PWD, WIFI_FIELD_SIZE, pwd);

    String mqttHost = preferences.getString("mqtt_host", "");
    if (mqttHost.length() == 0)
        mqttHost = DEFAULT_MQTT_HOST;

    String mqttTopic = preferences.getString("mqtt_topic", "");
    if (mqttTopic.length() == 0)
        mqttTopic = DEFAULT_MQTT_TOPIC;

    CopyStringField(data.mqttHost, MQTT_FIELD_SIZE, mqttHost);
    CopyStringField(data.mqttTopic, MQTT_FIELD_SIZE, mqttTopic);
    data.mqttPort = (uint16_t)preferences.getUInt("mqtt_port", DEFAULT_MQTT_PORT);

    preferences.end();

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        autoRestoreAt[i] = 0;
        autoPaused[i] = false;
    }

    NormalizeThresholds();
    ApplyMotorAutoFlags();
}

void SaveSettings()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (!autoPaused[i])
        {
            data.ao[i] = MotorDriver[i]->AutoOpen;
            data.ac[i] = MotorDriver[i]->AutoClose;
        }
    }

    NormalizeThresholds();

    preferences.begin("config", false);

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        String closeKey = "c" + String(i + 1);
        String openKey = "o" + String(i + 1);
        String acKey = "ac" + String(i + 1);
        String aoKey = "ao" + String(i + 1);
        String arKey = "ar" + String(i + 1);

        preferences.putFloat(closeKey.c_str(), data.c[i]);
        preferences.putFloat(openKey.c_str(), data.o[i]);
        preferences.putBool(acKey.c_str(), data.ac[i]);
        preferences.putBool(aoKey.c_str(), data.ao[i]);
        preferences.putUInt(arKey.c_str(), data.ar[i]);
    }

    preferences.end();

    ApplyMotorAutoFlags();
    MqttPublishAllMotors();
}

void SaveWiFiSettings()
{
    preferences.begin("config", false);
    preferences.putString("wifi_ssid", data.SSID);
    preferences.putString("wifi_pass", data.PWD);
    preferences.end();

    WiFiCtrl->ApplySettingsFromNvs();
}

void SaveMqttSettings()
{
    if (data.mqttPort == 0)
        data.mqttPort = DEFAULT_MQTT_PORT;

    preferences.begin("config", false);
    preferences.putString("mqtt_host", data.mqttHost);
    preferences.putUInt("mqtt_port", data.mqttPort);
    preferences.putString("mqtt_topic", data.mqttTopic);
    preferences.end();

    ApplyMqttSettings();
}

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

void TickAutoRestore()
{
    unsigned long now = millis();

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (autoRestoreAt[i] == 0)
            continue;

        if (now < autoRestoreAt[i])
            continue;

        autoRestoreAt[i] = 0;
        autoPaused[i] = false;
        MotorDriver[i]->AutoOpen = data.ao[i];
        MotorDriver[i]->AutoClose = data.ac[i];
        MqttPublishMotor(i);
    }
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

static bool IsCloseAction(MotorAction action)
{
    return action == MotorAction::Close || action == MotorAction::InitClose;
}

static bool IsSameDirection(MotorAction a, MotorAction b)
{
    if (a == b)
        return true;

    return IsCloseAction(a) && IsCloseAction(b);
}

static bool motorOtaPaused{false};

static void ClearActiveMotor()
{
    activeMotorIndex = -1;
    activeMotorAction = MotorAction::None;
}

static int FindQueueEntryForMotor(int index)
{
    for (int i = 0; i < motorQueueCount; i++)
    {
        if (motorQueue[i].index == index)
            return i;
    }

    return -1;
}

static void RemoveQueueEntryForMotor(int index)
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

static void PopQueueFront()
{
    if (motorQueueCount <= 0)
        return;

    for (int i = 0; i < motorQueueCount - 1; i++)
        motorQueue[i] = motorQueue[i + 1];

    motorQueueCount--;
}

static bool ExecuteMotorAction(int index, MotorAction action)
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

static void EnqueueMotorAction(int index, MotorAction action)
{
    if (motorQueueCount >= MOTOR_COUNT)
        return;

    motorQueue[motorQueueCount].index = (int8_t)index;
    motorQueue[motorQueueCount].action = action;
    motorQueueCount++;
}

static void RequestMotorAction(int index, MotorAction action)
{
    if (motorOtaPaused)
        return;

    if (index < 0 || index >= MOTOR_COUNT || action == MotorAction::None)
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

void RequestMotorOpen(int index)
{
    RequestMotorAction(index, MotorAction::Open);
}

void RequestMotorClose(int index)
{
    RequestMotorAction(index, MotorAction::Close);
}

void InitCloseAllMotors()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
        RequestMotorAction(i, MotorAction::InitClose);
}

void PauseMotorsForOta()
{
    motorOtaPaused = true;

    if (activeMotorIndex >= 0)
    {
        MotorDriver[activeMotorIndex]->Stop();
        ClearActiveMotor();
    }

    motorQueueCount = 0;
}

void ResumeMotorsAfterOta()
{
    motorOtaPaused = false;
}

void ProcessMotorQueue()
{
    if (motorOtaPaused)
        return;

    if (activeMotorIndex >= 0)
    {
        if (MotorDriver[activeMotorIndex]->IsBusy())
            return;

        ClearActiveMotor();
    }

    while (motorQueueCount > 0)
    {
        const int index = motorQueue[0].index;
        const MotorAction action = motorQueue[0].action;
        PopQueueFront();

        if (ExecuteMotorAction(index, action))
            return;
    }
}

void ManualOpen(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    RequestMotorOpen(index);
    PauseAutoControl(index);
    MqttPublishMotor(index);
}

void ManualClose(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    RequestMotorClose(index);
    PauseAutoControl(index);
    MqttPublishMotor(index);
}

void ProcessMotorAutomation(float temp)
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (autoPaused[i])
            continue;

        if (data.ao[i] && !MotorDriver[i]->IsOpen() && temp > data.o[i])
        {
            RequestMotorOpen(i);
            MqttPublishMotor(i);
        }

        if (data.ac[i] && MotorDriver[i]->IsOpen() && temp < data.c[i] - TEMP_HYSTERESIS)
        {
            RequestMotorClose(i);
            MqttPublishMotor(i);
        }
    }
}
