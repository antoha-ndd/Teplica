#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "var.h"
#include "mqtt.h"
#include "settings.h"
#include "storage.h"
#include "MotorQueueControl.h"
#include "MotorDriver.h"
#include "bmp180.h"
#include "app/MQTTControl.h"

static bool payloadTrue(const String &payload)
{
    return payload == "1" || payload == "ON" || payload == "on" ||
           payload == "true" || payload == "open";
}

static bool stripTopicPrefix(String &topic)
{
    String prefix = String(data.mqttTopic) + "/";
    if (!topic.startsWith(prefix))
        return false;

    topic = topic.substring(prefix.length());
    return true;
}

static int parseMotorIndex(const String &segment)
{
    if (segment.length() != 1 || segment[0] < '0' || segment[0] > '9')
        return -1;

    int index = segment[0] - '0';
    if (index < 0 || index >= MOTOR_COUNT)
        return -1;

    return index;
}

void InitMqtt()
{
    MQTTCtrl = new TMQTTControl();
    MQTTCtrl->MQTT_Timeout = 3000;
    MQTTCtrl->Register(App);
    ApplyMqttSettings();
}

void ApplyMqttSettings()
{
    if (!MQTTCtrl)
        return;

    String topic = data.mqttTopic;
    if (topic.length() == 0)
        topic = DEFAULT_MQTT_TOPIC;

    MQTTCtrl->InitMQTT(String(data.mqttHost), data.mqttPort, topic);
}

String GetMqttStatusText()
{
    if (!MQTTCtrl)
        return "-";

    if (MQTTCtrl->IsMQTTConnected())
        return "подключён";

    if (WiFi.isConnected())
        return "нет связи";

    return "ожидание WiFi";
}

void MqttPublishMotor(int index)
{
    if (!MQTTCtrl || !MQTTCtrl->IsMQTTConnected() || index < 0 || index >= MOTOR_COUNT)
        return;

    char topic[64];
    char value[16];

    snprintf(topic, sizeof(topic), "motor/%d/state", index);
    MQTTCtrl->PublishUnderTopic(topic,
                                !data.motorEnabled[index] ? "disabled" :
                                MotorDriver[index]->IsOpen() ? "open" : "closed", true);

    int rawState;
    if (!data.motorEnabled[index])
        rawState = 0;
    else if (TMotorQueueControl::ActiveMotorIndex() == index)
        rawState = (TMotorQueueControl::ActiveMotorAction() == MotorAction::Open) ? 2 : 3;
    else if (MotorDriver[index]->IsOpen())
        rawState = 4;
    else
        rawState = 5;

    snprintf(topic, sizeof(topic), "motor/%d/state_raw", index);
    snprintf(value, sizeof(value), "%d", rawState);
    MQTTCtrl->PublishUnderTopic(topic, value, true);

    snprintf(topic, sizeof(topic), "motor/%d/auto_open", index);
    MQTTCtrl->PublishUnderTopic(topic,
                                MotorDriver[index]->AutoOpen ? "1" : "0", true);
    snprintf(topic, sizeof(topic), "motor/%d/auto_close", index);
    MQTTCtrl->PublishUnderTopic(topic,
                                MotorDriver[index]->AutoClose ? "1" : "0", true);
    snprintf(topic, sizeof(topic), "motor/%d/threshold_open", index);
    MQTTCtrl->PublishUnderTopic(topic, String(data.o[index], 1), true);
    snprintf(topic, sizeof(topic), "motor/%d/threshold_close", index);
    MQTTCtrl->PublishUnderTopic(topic, String(data.c[index], 1), true);
    snprintf(topic, sizeof(topic), "motor/%d/auto_restore_min", index);
    MQTTCtrl->PublishUnderTopic(topic, String(data.ar[index]), true);
    snprintf(topic, sizeof(topic), "motor/%d/auto_paused", index);
    MQTTCtrl->PublishUnderTopic(topic, autoPaused[index] ? "1" : "0", true);
    snprintf(topic, sizeof(topic), "motor/%d/auto_restore_left", index);
    MQTTCtrl->PublishUnderTopic(topic, GetAutoRestoreStatusText(index), true);
    snprintf(topic, sizeof(topic), "motor/%d/motor_enabled", index);
    MQTTCtrl->PublishUnderTopic(topic, data.motorEnabled[index] ? "1" : "0", true);
}

void MqttPublishAllMotors()
{
    for (int i = 0; i < MOTOR_COUNT; i++)
        MqttPublishMotor(i);
}

void MqttPublishTelemetry()
{
    if (!MQTTCtrl || !MQTTCtrl->IsMQTTConnected())
        return;

    if (bmp->IsOk())
        MQTTCtrl->PublishUnderTopic("temperature", String(bmp->Temperature(true), 2), true);
    MQTTCtrl->PublishUnderTopic("rssi", String(WiFi.RSSI()), true);
    MQTTCtrl->PublishUnderTopic("wifi/connected", WiFi.isConnected() ? "1" : "0", true);
    MQTTCtrl->PublishUnderTopic("system/heap_free", String(ESP.getFreeHeap()), true);
}

void MqttPublishFullState()
{
    MqttPublishTelemetry();
    MqttPublishAllMotors();
}

void AppMQTTProcessMessage(String topic, String payload)
{
    if (!stripTopicPrefix(topic))
        return;

    if (!topic.startsWith("motor/"))
        return;

    int slash1 = topic.indexOf('/');
    int slash2 = topic.indexOf('/', slash1 + 1);
    if (slash2 < 0)
        return;

    int index = parseMotorIndex(topic.substring(slash1 + 1, slash2));
    if (index < 0)
        return;

    String action = topic.substring(slash2 + 1);

    if (action == "command/open")
    {
        TMotorQueueControl::ManualOpen(index);
        return;
    }

    if (action == "command/close")
    {
        TMotorQueueControl::ManualClose(index);
        return;
    }

    if (action == "command/stop")
    {
        TMotorQueueControl::Stop(index);
        return;
    }

    if (!action.startsWith("set/"))
        return;

    String field = action.substring(4);

    if (field == "auto_open")
    {
        MotorDriver[index]->AutoOpen = payloadTrue(payload);
        ApplyAutoFlagsFromWeb(index);
    }
    else if (field == "auto_close")
    {
        MotorDriver[index]->AutoClose = payloadTrue(payload);
        ApplyAutoFlagsFromWeb(index);
    }
    else if (field == "threshold_open")
    {
        float val = payload.toFloat();
        if (val < -50.0f || val > 100.0f)
            return;
        data.o[index] = val;
        SaveSettings();
    }
    else if (field == "threshold_close")
    {
        float val = payload.toFloat();
        if (val < -50.0f || val > 100.0f)
            return;
        data.c[index] = val;
        SaveSettings();
    }
    else if (field == "auto_restore_min")
    {
        int val = payload.toInt();
        if (val < 0 || val > 1440)
            return;
        data.ar[index] = (uint16_t)val;
        SaveSettings();
    }
    else if (field == "motor_enabled")
    {
        data.motorEnabled[index] = payloadTrue(payload);
        SaveSettings();
    }
    else
    {
        return;
    }

    MqttPublishMotor(index);
}
