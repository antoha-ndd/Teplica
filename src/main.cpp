#include <Arduino.h>
#include "AppSettings.h"


void setup() {

  Serial.begin(57600);
  Serial.flush();
  Serial.println("");
  Serial.println("START");
 
  App = new TApplication();
  App->Run();

  bmp = new TBMP180(NULL);
  bmp->Register(App);

  Timer1  = new TTimer();
  Timer1->OnTimeout = Timer1_Timeout;
  Timer1->Register( App );
  Timer1->Start(1000);

  LCD = new TSSD1306();

  PCF = new TPCF8575();
  Btn1  = new TPCF8575_Button( PCF , P12);
  Btn1->Register(App);
  Btn1->OnClick = Btn1_OnClick;
  Btn1->OnPress = Btn1_OnPress;
  Btn1->OnRelease = Btn1_OnRelease;


}

void loop() {
  
    App->Idle();

}

