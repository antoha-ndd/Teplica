#include <Arduino.h>
#include <WiFi.h>
#include <SettingsGyver.h>

#include "config.h"
#include "var.h"
#include "webui.h"
#include "settings.h"
#include "MotorDriver.h"
#include "bmp180.h"

static SettingsGyver uiInstance("Teplica");
SettingsGyver *ui = &uiInstance;

static void buildMotorMenu(sets::Builder &b, int index, const char *title,
                           size_t opId, size_t cId, size_t oId,
                           size_t aoId, size_t acId,
                           size_t openId, size_t closeId)
{
    if (!b.beginMenu(title))
        return;

    b.LED(opId, "Открыто", MotorDriver[index]->IsOpen(),
         sets::Colors::Red, sets::Colors::Green);
    b.Number(cId, "Закрытие", &data.c[index]);
    b.Number(oId, "Открытие", &data.o[index]);
    b.Switch(aoId, "Автооткрытие", &data.ao[index]);
    b.Switch(acId, "Автозакрытие", &data.ac[index]);

    if (b.beginButtons())
    {
        if (b.Button(openId, "Открыть"))
            ManualOpen(index);

        if (b.Button(closeId, "Закрыть"))
            MotorDriver[index]->Close();

        b.endButtons();
    }

    b.endMenu();
}

void buildUi(sets::Builder &b)
{
    b.LabelFloat(uiid::Temp, "Температура", bmp->Temperature(true));
    b.LabelNum(uiid::Rssi, "RSSI", WiFi.RSSI());

    buildMotorMenu(b, 0, "Окно ближнее",
                   uiid::Op1, uiid::C1, uiid::O1, uiid::Ao1, uiid::Ac1,
                   uiid::Open1, uiid::Close1);

    buildMotorMenu(b, 1, "Окно дальнее (у бочки)",
                   uiid::Op2, uiid::C2, uiid::O2, uiid::Ao2, uiid::Ac2,
                   uiid::Open2, uiid::Close2);

    buildMotorMenu(b, 2, "Дверь дальння (на дорогу)",
                   uiid::Op3, uiid::C3, uiid::O3, uiid::Ao3, uiid::Ac3,
                   uiid::Open3, uiid::Close3);

    if (b.beginMenu("WI-FI"))
    {
        b.Input(uiid::SSID, "Имя сети", AnyPtr(data.SSID, WIFI_FIELD_SIZE));
        b.Pass(uiid::PWD, "Пароль", AnyPtr(data.PWD, WIFI_FIELD_SIZE));

        if (b.Button(uiid::ApplyWiFi, "Применить"))
            SaveWiFiSettings();

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
    case uiid::Reboot:
        return;
    default:
        SaveSettings();
        break;
    }
}

void updateUi(sets::Updater &upd)
{
    upd.update(uiid::Temp, bmp->Temperature(true));
    upd.update(uiid::Rssi, WiFi.RSSI());
    upd.update(uiid::Op1, MotorDriver[0]->IsOpen());
    upd.update(uiid::Op2, MotorDriver[1]->IsOpen());
    upd.update(uiid::Op3, MotorDriver[2]->IsOpen());
}

void InitWebUi()
{
    ui->config.updateTout = 500;
    ui->config.useFS = false;
    ui->begin(false, "teplica");
    ui->onBuild(buildUi);
    ui->onUpdate(updateUi);
}
