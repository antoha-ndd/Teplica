#pragma once

#include <cstddef>

#define SETT_NO_DB

constexpr int MOTOR_COUNT = 3;
constexpr size_t WIFI_FIELD_SIZE = 100;
constexpr float TEMP_HYSTERESIS = 0.5f;

namespace uiid
{
    enum : size_t
    {
        Temp,
        Rssi,
        Op1,
        Op2,
        Op3,
        C1,
        C2,
        C3,
        O1,
        O2,
        O3,
        Ao1,
        Ao2,
        Ao3,
        Ac1,
        Ac2,
        Ac3,
        Open1,
        Open2,
        Open3,
        Close1,
        Close2,
        Close3,
        SSID,
        PWD,
        ApplyWiFi,
        Reboot,
    };
}
