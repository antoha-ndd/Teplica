#pragma once

#include <WString.h>

void InitMqtt();
void ApplyMqttSettings();
String GetMqttStatusText();
void MqttPublishFullState();
void MqttPublishTelemetry();
void MqttPublishAllMotors();
void MqttPublishMotor(int index);
