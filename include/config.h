#pragma once

#include <cstddef>

#define SETT_NO_DB

constexpr int MOTOR_COUNT = 3;
constexpr size_t WIFI_FIELD_SIZE = 100;
constexpr size_t MQTT_FIELD_SIZE = 64;
constexpr float TEMP_HYSTERESIS = 0.5f;
constexpr float DEFAULT_TEMP_OPEN = 21.0f;
constexpr float DEFAULT_TEMP_CLOSE = 20.0f;

constexpr char DEFAULT_MQTT_HOST[] = "192.168.0.51";
constexpr uint16_t DEFAULT_MQTT_PORT = 1883;
constexpr char DEFAULT_MQTT_TOPIC[] = "teplica";

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
        Ar1,
        Ar2,
        Ar3,
        ArLeft1,
        ArLeft2,
        ArLeft3,
        En1,
        En2,
        En3,
        Open1,
        Open2,
        Open3,
        Close1,
        Close2,
        Close3,
        SSID,
        PWD,
        ApplyWiFi,
        MqttHost,
        MqttPort,
        MqttTopic,
        MqttStatus,
        ApplyMqtt,
        Reboot,
    };
}
