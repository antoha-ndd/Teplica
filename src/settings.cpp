#include <Arduino.h>
#include <cstring>

#include "config.h"
#include "var.h"
#include "settings.h"
#include "MotorDriver.h"
#include "app/WiFiControl.h"

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

        data.c[i] = preferences.getFloat(closeKey.c_str(), 0);
        data.o[i] = preferences.getFloat(openKey.c_str(), 0);
        data.ac[i] = preferences.getBool(acKey.c_str(), false);
        data.ao[i] = preferences.getBool(aoKey.c_str(), false);
    }

    String ssid = preferences.getString("wifi_ssid", "");
    if (ssid.length() == 0)
        ssid = preferences.getString("SSID", "");

    String pwd = preferences.getString("wifi_pass", "");
    if (pwd.length() == 0)
        pwd = preferences.getString("PWD", "");

    CopyStringField(data.SSID, WIFI_FIELD_SIZE, ssid);
    CopyStringField(data.PWD, WIFI_FIELD_SIZE, pwd);

    preferences.end();

    NormalizeThresholds();
    ApplyMotorAutoFlags();
}

void SaveSettings()
{
    NormalizeThresholds();

    preferences.begin("config", false);

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        String closeKey = "c" + String(i + 1);
        String openKey = "o" + String(i + 1);
        String acKey = "ac" + String(i + 1);
        String aoKey = "ao" + String(i + 1);

        preferences.putFloat(closeKey.c_str(), data.c[i]);
        preferences.putFloat(openKey.c_str(), data.o[i]);
        preferences.putBool(acKey.c_str(), data.ac[i]);
        preferences.putBool(aoKey.c_str(), data.ao[i]);
    }

    preferences.end();

    ApplyMotorAutoFlags();
}

void SaveWiFiSettings()
{
    preferences.begin("config", false);
    preferences.putString("wifi_ssid", data.SSID);
    preferences.putString("wifi_pass", data.PWD);
    preferences.end();

    WiFiCtrl->ApplySettingsFromNvs();
}

void ManualOpen(int index)
{
    if (index < 0 || index >= MOTOR_COUNT)
        return;

    MotorDriver[index]->Open();
    data.ac[index] = false;
    SaveSettings();
}
