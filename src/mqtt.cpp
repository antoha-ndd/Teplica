#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "var.h"
#include "mqtt.h"
#include "settings.h"
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

    String base = "motor/" + String(index) + "/";

    MQTTCtrl->PublishUnderTopic(base + "state",
                                MotorDriver[index]->IsOpen() ? "open" : "closed", true);
    MQTTCtrl->PublishUnderTopic(base + "auto_open",
                                MotorDriver[index]->AutoOpen ? "1" : "0", true);
    MQTTCtrl->PublishUnderTopic(base + "auto_close",
                                MotorDriver[index]->AutoClose ? "1" : "0", true);
    MQTTCtrl->PublishUnderTopic(base + "threshold_open", String(data.o[index], 1), true);
    MQTTCtrl->PublishUnderTopic(base + "threshold_close", String(data.c[index], 1), true);
    MQTTCtrl->PublishUnderTopic(base + "auto_restore_min", String(data.ar[index]), true);
    MQTTCtrl->PublishUnderTopic(base + "auto_paused", autoPaused[index] ? "1" : "0", true);
    MQTTCtrl->PublishUnderTopic(base + "auto_restore_left",
                                GetAutoRestoreStatusText(index), true);
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
        ManualOpen(index);
        MqttPublishMotor(index);
        return;
    }

    if (action == "command/close")
    {
        ManualClose(index);
        MqttPublishMotor(index);
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
        data.o[index] = payload.toFloat();
        SaveSettings();
    }
    else if (field == "threshold_close")
    {
        data.c[index] = payload.toFloat();
        SaveSettings();
    }
    else if (field == "auto_restore_min")
    {
        data.ar[index] = (uint16_t)payload.toInt();
        SaveSettings();
    }
    else
    {
        return;
    }

    MqttPublishMotor(index);
}
