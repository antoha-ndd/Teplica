#pragma once

#include <WString.h>

void InitMqtt();
void MqttPublishFullState();
void MqttPublishTelemetry();
void MqttPublishAllMotors();
void MqttPublishMotor(int index);
