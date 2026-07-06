#include <Arduino.h>
#include <cstring>

#include "config.h"
#include "var.h"
#include "storage.h"
#include "settings.h"
#include "mqtt.h"
#include "MotorDriver.h"
#include "app/WiFiControl.h"

void NormalizeThresholds()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        if (data.o[i] <= data.c[i])
        {
            Serial.printf("[WARN] Motor %d: open threshold (%.1f) <= close threshold (%.1f), "
                          "correcting to %.1f\n",
                          i, data.o[i], data.c[i], data.c[i] + TEMP_HYSTERESIS * 2.0f);
            data.o[i] = data.c[i] + TEMP_HYSTERESIS * 2.0f;
        }
    }
}

void CopyStringField(char *dest, size_t size, const String &src)
{
    if (size == 0)
        return;

    strncpy(dest, src.c_str(), size - 1);
    dest[size - 1] = '\0';
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
        String enabledKey = "en" + String(i + 1);
        data.motorEnabled[i] = preferences.isKey(enabledKey.c_str())
                                   ? preferences.getBool(enabledKey.c_str(), true)
                                   : true;
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
        String enabledKey = "en" + String(i + 1);
        preferences.putBool(enabledKey.c_str(), data.motorEnabled[i]);
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
    if (data.mqttPort == 0 || data.mqttPort > 65535)
        data.mqttPort = DEFAULT_MQTT_PORT;

    preferences.begin("config", false);
    preferences.putString("mqtt_host", data.mqttHost);
    preferences.putUInt("mqtt_port", data.mqttPort);
    preferences.putString("mqtt_topic", data.mqttTopic);
    preferences.end();

    ApplyMqttSettings();
}
