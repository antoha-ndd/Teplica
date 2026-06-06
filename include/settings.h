#pragma once

#include <WString.h>

void LoadSettings();
void SaveSettings();
void NormalizeThresholds();
void ApplyMotorAutoFlags();
void SaveWiFiSettings();
void SaveMqttSettings();
void ManualOpen(int index);
void ManualClose(int index);
void PauseAutoControl(int index);
void TickAutoRestore();
void ProcessMotorAutomation(float temp);
void ApplyAutoFlagsFromWeb(int index);
String GetAutoRestoreStatusText(int index);
void CopyStringField(char *dest, size_t size, const String &src);
