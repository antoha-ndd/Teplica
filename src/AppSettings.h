
#define GH_NO_PAIRS

#include <espwifi.h>
#include "Objects.h"
#include "bmp180.h"
#include "ObjectTimer.h"
#include "ssd1306.h"
#include "button.h"
#include "varbasetypes.h"
#include "MotorDriver.h"
#include "websettings.h"

TApplication *App;
TBMP180 *bmp;
TTimer *Timer1, *Timer2;
TSSD1306 *LCD;
TButton *BtnOpen[3];
TButton *BtnClose[3];
TMotorDriver *MotorDriver[3];
ConfigWebServer configServer(80);


void OnOTAProgress(unsigned int Progress, unsigned int Total)
{

    static int Pr = 0;
    int CurrentPr = (Progress * 100) / Total;

    if (CurrentPr != Pr)
    {
        Pr = CurrentPr;
        LCD->clearDisplay();
        LCD->setCursor(10, 1);
        LCD->setTextSize(1);
        LCD->print("Updating firmware");
        LCD->setCursor(45, 15);
        LCD->setTextSize(2);
        LCD->print(String(Pr) + "%");
        LCD->display();
    }
}

void BtnOpen1_Click(TButton *Button)
{

    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Open 1");
    LCD->display();

    MotorDriver[0]->Open();
};

void BtnClose1_Click(TButton *Button)
{

    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Close 1");
    LCD->display();

    MotorDriver[0]->Close();
};

void BtnOpen2_Click(TButton *Button)
{

    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Open 2");
    LCD->display();

    MotorDriver[1]->Open();
};

void BtnClose2_Click(TButton *Button)
{
    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Close 2");
    LCD->display();

    MotorDriver[1]->Close();
};

void BtnOpen3_Click(TButton *Button)
{

    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Open 3");
    LCD->display();

    MotorDriver[2]->Open();
};

void BtnClose3_Click(TButton *Button)
{
    LCD->clearDisplay();
    LCD->setCursor(1, 5);
    LCD->setTextSize(2);
    LCD->print("Close 3");
    LCD->display();

    MotorDriver[2]->Close();
};

void Timer1_Timeout(TTimer *Timer)
{

    App->PrintLn(String(bmp->Temperature(true)));

    LCD->clearDisplay();
    LCD->setCursor(20, 5);
    LCD->setTextSize(3);
    LCD->print(String(bmp->Temperature()).c_str());
    LCD->display();
};

void Timer2_Timeout(TTimer *Timer)
{
    float temp = bmp->Temperature(true);
        
    for (int i = 0; i < 3; i++)
    {
        float Ct = configServer.getValue("Min"+String(i+1)).toFloat();
        float Ot = configServer.getValue("Max"+String(i+1)).toFloat();
    Ot = 15;
        if (MotorDriver[i]->AutoOpen && Ot<temp) MotorDriver[i]->Open();
        if (MotorDriver[i]->AutoClose && !isnanf(Ct) && MotorDriver[i]->IsOpen() && Ct>temp) MotorDriver[i]->Close();
    }
}

void Init()
{
    ArduinoOTA.onProgress(OnOTAProgress);
    App = new TApplication();
    App->Run();

    LCD = new TSSD1306();
    LCD->clearDisplay();
    LCD->setCursor(20, 5);
    LCD->setTextSize(3);
    LCD->print("START");
    LCD->display();
    delay(1000);

    bmp = new TBMP180(NULL);
    bmp->Register(App);

    Timer1 = new TTimer();
    Timer1->OnTimeout = Timer1_Timeout;
    Timer1->Register(App);
    Timer1->Start(5000);

    Timer2 = new TTimer();
    Timer2->OnTimeout = Timer2_Timeout;
    Timer2->Register(App);
    Timer2->Start(1000);

    BtnOpen[0] = new TButton(NULL, 5, false);
    BtnOpen[0]->OnPress = BtnOpen1_Click;
    BtnOpen[0]->Register(App);

    BtnOpen[1] = new TButton(NULL, 4, false);
    BtnOpen[1]->OnPress = BtnOpen2_Click;
    BtnOpen[1]->Register(App);

    BtnOpen[2] = new TButton(NULL, 18, false);
    BtnOpen[2]->OnPress = BtnOpen3_Click;
    BtnOpen[2]->Register(App);

    BtnClose[0] = new TButton(NULL, 19, false);
    BtnClose[0]->OnPress = BtnClose1_Click;
    BtnClose[0]->Register(App);

    BtnClose[1] = new TButton(NULL, 16, false);
    BtnClose[1]->OnPress = BtnClose2_Click;
    BtnClose[1]->Register(App);

    BtnClose[2] = new TButton(NULL, 17, false);
    BtnClose[2]->OnPress = BtnClose3_Click;
    BtnClose[2]->Register(App);

    configServer.fields = {
        
        ConfigWebServer::FieldDefinition("", "", true, "MQTT настройки"),
        ConfigWebServer::FieldDefinition("MQTTServer", "text", false, "Сервер"),
        ConfigWebServer::FieldDefinition("MQTTPort", "number", false, "Порт"),
        ConfigWebServer::FieldDefinition("MQTTTopic", "text", false, "Топик"),
        ConfigWebServer::FieldDefinition("", "", true, "Окно 1"),
        ConfigWebServer::FieldDefinition("Min1", "number", false, "Закрытие"),
        ConfigWebServer::FieldDefinition("Max1", "number", false, "Открытие"),
        ConfigWebServer::FieldDefinition("", "", true, "Окно 2"),
        ConfigWebServer::FieldDefinition("Min2", "number", false, "Закрытие"),
        ConfigWebServer::FieldDefinition("Max2", "number", false, "Открытие"),
        ConfigWebServer::FieldDefinition("", "", true, "Дверь"),
        ConfigWebServer::FieldDefinition("Min3", "number", false, "Закрытие"),
        ConfigWebServer::FieldDefinition("Max3", "number", false, "Открытие")

    };

    configServer.Register(App);

    MotorDriver[0] = new TMotorDriver(14, 27);
    MotorDriver[1] = new TMotorDriver(12, 13);
    MotorDriver[2] = new TMotorDriver(2, 26);

    MotorDriver[0]->Register(App);
    MotorDriver[1]->Register(App);
    MotorDriver[2]->Register(App);

    MotorDriver[0]->InitClose();
    MotorDriver[1]->InitClose();
    MotorDriver[2]->InitClose();

    MotorDriver[0]->AutoClose = true;
    MotorDriver[0]->AutoOpen = true;

    MotorDriver[1]->AutoClose = true;
    MotorDriver[1]->AutoOpen = true;

    MotorDriver[2]->AutoClose = true;
    MotorDriver[2]->AutoOpen = true;
}
// open 10 13 12
// close 8 9 11
