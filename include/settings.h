#pragma once

#include <WString.h>

void LoadSettings();
void SaveSettings();
void NormalizeThresholds();
void ApplyMotorAutoFlags();
void SaveWiFiSettings();
void ManualOpen(int index);
void CopyStringField(char *dest, size_t size, const String &src);
