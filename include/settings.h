#pragma once

#include <WString.h>

void ApplyMotorAutoFlags();
void PauseAutoControl(int index);
void ProcessMotorAutomation(float temp);
void ApplyAutoFlagsFromWeb(int index);
String GetAutoRestoreStatusText(int index);
