#include <Arduino.h>
#include <WiFi.h>
#include <SettingsGyver.h>

#include "config.h"
#include "var.h"
#include "webui.h"
#include "settings.h"
#include "storage.h"
#include "MotorQueueControl.h"
#include "MotorDriver.h"
#include "bmp180.h"
#include "mqtt.h"

static SettingsGyver uiInstance("Teplica");
SettingsGyver *ui = &uiInstance;

static int motorIndexFromAutoFlagId(size_t id)
{
    switch (id)
    {
    case uiid::Ao1:
    case uiid::Ac1:
    case uiid::En1:
        return 0;
    case uiid::Ao2:
    case uiid::Ac2:
    case uiid::En2:
        return 1;
    case uiid::Ao3:
    case uiid::Ac3:
    case uiid::En3:
        return 2;
    default:
        return -1;
    }
}

static void buildMotorMenu(sets::Builder &b, int index, const char *title,
                           size_t opId, size_t cId, size_t oId,
                           size_t aoId, size_t acId, size_t arId, size_t arLeftId,
                           size_t enId, size_t openId, size_t closeId)
{
    if (!b.beginMenu(title))
        return;

    b.LED(opId, "Открыто", MotorDriver[index]->IsOpen(),
         sets::Colors::Red, sets::Colors::Green);
    b.Number(cId, "Закрытие", &data.c[index]);
    b.Number(oId, "Открытие", &data.o[index]);
    b.Switch(aoId, "Автооткрытие", &MotorDriver[index]->AutoOpen);
    b.Switch(acId, "Автозакрытие", &MotorDriver[index]->AutoClose);
    b.Number(arId, "Восст. авто, мин", &data.ar[index], 0, 1440);
    b.Label(arLeftId, "Осталось, мин", GetAutoRestoreStatusText(index));
    b.Switch(enId, "Мотор включён", &data.motorEnabled[index]);

    if (b.beginButtons())
    {
        if (b.Button(openId, "Открыть"))
            TMotorQueueControl::ManualOpen(index);

        if (b.Button(closeId, "Закрыть"))
            TMotorQueueControl::ManualClose(index);

        b.endButtons();
    }

    b.endMenu();
}

void buildUi(sets::Builder &b)
{
    b.LabelFloat(uiid::Temp, "Температура",
                 bmp->IsOk() ? bmp->Temperature(true) : -999.0f);
    b.LabelNum(uiid::Rssi, "RSSI", WiFi.RSSI());

    buildMotorMenu(b, 0, "Окно ближнее",
                   uiid::Op1, uiid::C1, uiid::O1, uiid::Ao1, uiid::Ac1,
                   uiid::Ar1, uiid::ArLeft1, uiid::En1, uiid::Open1, uiid::Close1);

    buildMotorMenu(b, 1, "Окно дальнее (у бочки)",
                   uiid::Op2, uiid::C2, uiid::O2, uiid::Ao2, uiid::Ac2,
                   uiid::Ar2, uiid::ArLeft2, uiid::En2, uiid::Open2, uiid::Close2);

    buildMotorMenu(b, 2, "Дверь дальння (на дорогу)",
                   uiid::Op3, uiid::C3, uiid::O3, uiid::Ao3, uiid::Ac3,
                   uiid::Ar3, uiid::ArLeft3, uiid::En3, uiid::Open3, uiid::Close3);

    if (b.beginMenu("WI-FI"))
    {
        b.Input(uiid::SSID, "Имя сети", AnyPtr(data.SSID, WIFI_FIELD_SIZE));
        b.Pass(uiid::PWD, "Пароль", AnyPtr(data.PWD, WIFI_FIELD_SIZE));

        if (b.Button(uiid::ApplyWiFi, "Применить"))
            SaveWiFiSettings();

        b.endMenu();
    }

    if (b.beginMenu("MQTT"))
    {
        b.Input(uiid::MqttHost, "Брокер", AnyPtr(data.mqttHost, MQTT_FIELD_SIZE));
        b.Number(uiid::MqttPort, "Порт", &data.mqttPort, 1, 65535);
        b.Input(uiid::MqttTopic, "Топик", AnyPtr(data.mqttTopic, MQTT_FIELD_SIZE));
        b.Label(uiid::MqttStatus, "Статус", GetMqttStatusText());

        if (b.Button(uiid::ApplyMqtt, "Применить"))
            SaveMqttSettings();

        b.endMenu();
    }

    if (b.Button(uiid::Reboot, "Перезагрузить", sets::Colors::Red))
        ESP.restart();

    if (!b.build.isAction())
        return;

    switch (b.build.id)
    {
    case uiid::Open1:
    case uiid::Open2:
    case uiid::Open3:
    case uiid::Close1:
    case uiid::Close2:
    case uiid::Close3:
    case uiid::ApplyWiFi:
    case uiid::ApplyMqtt:
    case uiid::Reboot:
        return;
    case uiid::Ao1:
    case uiid::Ao2:
    case uiid::Ao3:
    case uiid::Ac1:
    case uiid::Ac2:
    case uiid::Ac3:
    case uiid::En1:
    case uiid::En2:
    case uiid::En3:
    {
        int index = motorIndexFromAutoFlagId(b.build.id);
        if (index >= 0)
            ApplyAutoFlagsFromWeb(index);
        return;
    }
    default:
        SaveSettings();
        break;
    }
}

void updateUi(sets::Updater &upd)
{
    upd.update(uiid::Temp, bmp->IsOk() ? bmp->Temperature(true) : -999.0f);
    upd.update(uiid::Rssi, WiFi.RSSI());
    upd.update(uiid::MqttStatus, GetMqttStatusText());
    upd.update(uiid::Op1, MotorDriver[0]->IsOpen());
    upd.update(uiid::Op2, MotorDriver[1]->IsOpen());
    upd.update(uiid::Op3, MotorDriver[2]->IsOpen());

    for (int i = 0; i < MOTOR_COUNT; i++)
    {
        upd.update(uiid::Ao1 + i, MotorDriver[i]->AutoOpen);
        upd.update(uiid::Ac1 + i, MotorDriver[i]->AutoClose);
        upd.update(uiid::ArLeft1 + i, GetAutoRestoreStatusText(i));
    }
}

void InitWebUi()
{
    ui->config.updateTout = 500;
    ui->config.useFS = false;
    ui->begin(false, "teplica");
    ui->onBuild(buildUi);
    ui->onUpdate(updateUi);
}
