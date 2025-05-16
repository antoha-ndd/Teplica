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
  Timer1->Start(5000);

  LCD = new TSSD1306();

  PCF = new TPCF8575();

  Btn1  = new TPCF8575_Button( PCF , P10);
  Btn1->Register(App);
  Btn1->OnClick = Btn1_OnClick;
  Btn1->OnPress = Btn1_OnPress;
  Btn1->OnRelease = Btn1_OnRelease;

  Btn2  = new TPCF8575_Button( PCF , P8);
  Btn2->Register(App);
  Btn2->OnClick = Btn1_OnClick;
  Btn2->OnPress = Btn1_OnPress;
  Btn2->OnRelease = Btn1_OnRelease;

  Btn3  = new TPCF8575_Button( PCF , P13);
  Btn3->Register(App);
  Btn3->OnClick = Btn1_OnClick;
  Btn3->OnPress = Btn1_OnPress;
  Btn3->OnRelease = Btn1_OnRelease;

  Btn4  = new TPCF8575_Button( PCF , P9);
  Btn4->Register(App);
  Btn4->OnClick = Btn1_OnClick;
  Btn4->OnPress = Btn1_OnPress;
  Btn4->OnRelease = Btn1_OnRelease;

  Btn5  = new TPCF8575_Button( PCF , P12);
  Btn5->Register(App);
  Btn5->OnClick = Btn1_OnClick;
  Btn5->OnPress = Btn1_OnPress;
  Btn5->OnRelease = Btn1_OnRelease;

  Btn6  = new TPCF8575_Button( PCF , P11);
  Btn6->Register(App);
  Btn6->OnClick = Btn1_OnClick;
  Btn6->OnPress = Btn1_OnPress;
  Btn6->OnRelease = Btn1_OnRelease;

  Od1  = new TPCF8575_OutputDevice( PCF , P1);
  Od1->Register(App);

  Od2  = new TPCF8575_OutputDevice( PCF , P2);
  Od2->Register(App);

  Od3  = new TPCF8575_OutputDevice( PCF , P3);
  Od3->Register(App);

  Od4  = new TPCF8575_OutputDevice( PCF , P4);
  Od4->Register(App);

  Od5  = new TPCF8575_OutputDevice( PCF , P5);
  Od5->Register(App);

  Od6  = new TPCF8575_OutputDevice( PCF , P6);
  Od6->Register(App);

}

void loop() {
  
    App->Idle();

}

