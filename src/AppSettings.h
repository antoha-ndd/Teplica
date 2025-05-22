
#define GH_NO_PAIRS

#include <espwifi.h>
#include "Objects.h"
#include "bmp180.h"
#include "ObjectTimer.h"
#include "ssd1306.h"
#include "button.h"
#include "varbasetypes.h"
#include "MotorDriver.h"
#include <GyverPortal.h>
#include <Preferences.h>

GyverPortal ui;
Preferences preferences;

TApplication *App;
TBMP180 *bmp;
TTimer *Timer1, *Timer2;
TSSD1306 *LCD;
TButton *BtnOpen[3];
TButton *BtnClose[3];
TMotorDriver *MotorDriver[3];
// ConfigWebServer configServer(80);

struct Data
{
    int Port;
    char MQTTServer[100];
    char MQTTTopic[100];
    float o[3], c[3];
    bool ac[3], ao[3];
};
Data data;

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
        if (MotorDriver[i]->AutoOpen && data.o[i] < temp)
            MotorDriver[i]->Open();

        if (MotorDriver[i]->AutoClose && data.c[i] > temp)
            MotorDriver[i]->Close();
    }
}

void LoadSettings(){
    preferences.begin("config", false);

    data.c[0] = preferences.getFloat("c1", 0);
    data.c[1] = preferences.getFloat("c2", 0);
    data.c[2] = preferences.getFloat("c3", 0);
    data.o[0] = preferences.getFloat("o1", 0);
    data.o[1] = preferences.getFloat("o2", 0);
    data.o[2] = preferences.getFloat("o3", 0);

    data.ac[0] = preferences.getBool("ac1", false);
    data.ac[1] = preferences.getBool("ac2", false);
    data.ac[2] = preferences.getBool("ac3", false);

    data.ao[0] = preferences.getBool("ao1", false);
    data.ao[1] = preferences.getBool("ao2", false);
    data.ao[2] = preferences.getBool("ao3", false);

    data.Port = preferences.getInt("Port", 1883);
    strcpy(data.MQTTServer, preferences.getString("Server", "").c_str());
    strcpy(data.MQTTTopic, preferences.getString("Topic", "").c_str());


    MotorDriver[0]->AutoClose = data.ac[0];
    MotorDriver[1]->AutoClose = data.ac[1];
    MotorDriver[2]->AutoClose = data.ac[2];

    MotorDriver[0]->AutoOpen = data.ao[0];
    MotorDriver[1]->AutoOpen = data.ao[1];
    MotorDriver[2]->AutoOpen = data.ao[2];

    preferences.end();
}

void build()
{

    LoadSettings();

    GP.BUILD_BEGIN(GP_DARK, 500);

    GP.LABEL("Температура : " + String(bmp->Temperature(true)));
    GP.BREAK();

    GP.BUTTON("SaveBtn", "Сохранить");

    GP_MAKE_BLOCK_TAB("Окно 1",

                      GP_MAKE_BOX(
                          GP.LABEL("Открыто");
                          GP.SWITCH("", true, GP_GREEN, true););

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c1", "", data.c[0]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o1", "", data.o[0])););

                      GP_MAKE_BOX(

                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao1", data.ao[0]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac1", data.ac[0]));

                      );

                      GP_MAKE_BOX(
                          GP.BUTTON("Open1", "Открыть");
                          GP.BUTTON("Close1", "Закрыть"););

    );

    GP_MAKE_BLOCK_TAB("Окно 2",

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c2", "", data.c[1]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o2", "", data.o[1])););

                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao2", data.ao[1]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac2", data.ac[1])););

                      GP_MAKE_BOX(
                          GP.BUTTON("Open2", "Открыть");
                          GP.BUTTON("Close2", "Закрыть");););

    GP_MAKE_BLOCK_TAB("Дверь",
                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Закрытие");
                              GP.NUMBER_F("c3", "", data.c[2]););

                          GP_MAKE_BOX(
                              GP.LABEL("Открытие");
                              GP.NUMBER_F("o3", "", data.o[2])););
                      GP_MAKE_BOX(
                          GP_MAKE_BOX(
                              GP.LABEL("Автооткрытие");
                              GP.SWITCH("ao3", data.ao[2]););

                          GP_MAKE_BOX(
                              GP.LABEL("Автозакрытие");
                              GP.SWITCH("ac3", data.ac[2])););
                      GP_MAKE_BOX(
                          GP.BUTTON("Open3", "Открыть");
                          GP.BUTTON("Close3", "Закрыть"););

    );

    GP_MAKE_BLOCK_TAB("MQTT",
                      GP_MAKE_BOX(
                          GP.LABEL("Cервер");
                          GP.TEXT("MQTTServer", "", data.MQTTServer););

                      GP_MAKE_BOX(
                          GP.LABEL("Порт");
                          GP.NUMBER("MQTTPort", "", data.Port));

                      GP_MAKE_BOX(
                          GP.LABEL("Топик");
                          GP.TEXT("MQTTTopic", "", data.MQTTTopic);););

    GP.BUTTON("RebootBtn", "Перезагрузить");

    GP.BUILD_END();


}

void action()
{
    if (ui.click())
    {
        // по клику переписать пришедшие данные в переменные
        ui.clickFloat("c1", data.c[0]);
        ui.clickFloat("c2", data.c[1]);
        ui.clickFloat("c3", data.c[2]);
        ui.clickFloat("o1", data.o[0]);
        ui.clickFloat("o2", data.o[1]);
        ui.clickFloat("o3", data.o[2]);

        ui.clickBool("ao1", data.ao[0]);
        ui.clickBool("ao2", data.ao[1]);
        ui.clickBool("ao3", data.ao[2]);

        ui.clickBool("ac1", data.ac[0]);
        ui.clickBool("ac2", data.ac[1]);
        ui.clickBool("ac3", data.ac[2]);

        ui.clickStr("MQTTServer", data.MQTTServer);
        ui.clickStr("MQTTTopic", data.MQTTTopic);
        ui.clickInt("MQTTPort", data.Port);

        if (ui.click("SaveBtn"))
        {

            preferences.begin("config", false);
            preferences.putFloat("c1", data.c[0]);
            preferences.putFloat("c2", data.c[1]);
            preferences.putFloat("c3", data.c[2]);
            preferences.putFloat("o1", data.o[0]);
            preferences.putFloat("o2", data.o[1]);
            preferences.putFloat("o3", data.o[2]);

            preferences.putBool("ac1", data.ac[0]);
            preferences.putBool("ac2", data.ac[1]);
            preferences.putBool("ac3", data.ac[2]);

            preferences.putBool("ao1", data.ao[0]);
            preferences.putBool("ao2", data.ao[1]);
            preferences.putBool("ao3", data.ao[2]);

            preferences.putInt("Port", data.Port);
            preferences.putString("Server", data.MQTTServer);
            preferences.putString("Topic", data.MQTTTopic);

            preferences.end();

            MotorDriver[0]->AutoClose = data.ac[0];
            MotorDriver[1]->AutoClose = data.ac[1];
            MotorDriver[2]->AutoClose = data.ac[2];
        
            MotorDriver[0]->AutoOpen = data.ao[0];
            MotorDriver[1]->AutoOpen = data.ao[1];
            MotorDriver[2]->AutoOpen = data.ao[2];
        }

        if (ui.click("RebootBtn"))
            ESP.restart();

        if (ui.click("Open1"))
            MotorDriver[0]->Open();

        if (ui.click("Open2"))
            MotorDriver[1]->Open();

        if (ui.click("Open3"))
            MotorDriver[2]->Open();

        if (ui.click("Close1"))
            MotorDriver[0]->Close();

        if (ui.click("Close2"))
            MotorDriver[1]->Close();

        if (ui.click("Close3"))
            MotorDriver[2]->Close();
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

    ui.start();
    ui.attachBuild(build);
    ui.attach(action);


    LoadSettings();
}
// open 10 13 12
// close 8 9 11
