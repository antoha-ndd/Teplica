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

void ManualOpen(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    MotorDriver[index]->Open();
    PauseAutoControl(index);
    MqttPublishMotor(index);
}

void ManualClose(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    MotorDriver[index]->Close();
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
            MotorDriver[i]->Open();
            MqttPublishMotor(i);
        }

        if (data.ac[i] && MotorDriver[i]->IsOpen() && temp < data.c[i] - TEMP_HYSTERESIS)
        {
            MotorDriver[i]->Close();
            MqttPublishMotor(i);
        }
    }
}
