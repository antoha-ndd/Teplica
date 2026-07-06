#pragma once

#include <WString.h>

void LoadSettings();
void SaveSettings();
void SaveWiFiSettings();
void SaveMqttSettings();
void NormalizeThresholds();
void CopyStringField(char *dest, size_t size, const String &src);
