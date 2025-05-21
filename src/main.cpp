
#include <Arduino.h>
#include "AppSettings.h"

void setup()
{
  Serial.begin(57600);
  Init();
}

void loop()
{

  App->Idle();
   configServer.handleClient();
}
