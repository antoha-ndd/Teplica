
#include <Arduino.h>
#include <SettingsGyver.h>
#include "var.h"
#include "webui.h"
#include "AppSettings.h"

void setup()
{
    Init();
}

void loop()
{
    App->Idle();
    ui->tick();
}
