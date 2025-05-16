#pragma once
#include "Objects.h"

#include <EEPROM.h>
#include <ctype.h>
#include <ArduinoJson.h>

class TSettings : public TControl
{
private:
    void ParseSettings(String JSON)
    {
        Fields.clear();
        DeserializationError error = deserializeJson(Fields, JSON);
        if (error)
        {
            Serial.println("JSON error");
            return;
        }
    }

public:
    JsonDocument Fields;

    void ClearSettings()
    {
        Fields.clear();
    }

    void AddField(String Key, String Value)
    {
        Fields[Key] = Value;
    }

    void ReadSettings()
    {
        String JSON = "";
        for (size_t i = 0; i < EEPROM.length(); i++)
        {
            char Ch = EEPROM.read(i);
            if (!isprint(Ch))
                break;
            JSON.concat(Ch);
        }

        Serial.println("JSON : [" + JSON + "]");
        ParseSettings(JSON);
    }

    void SaveSettings()
    {
        String JSON = "";
        serializeJson(Fields, JSON);

        for (size_t i = 0; i < JSON.length(); i++)
            EEPROM.write(i, JSON[i]);
        for (size_t i = JSON.length(); i < EEPROM.length(); i++)
            EEPROM.write(i, 0);

#if defined(ESP8266)
        EEPROM.commit();
#endif
    }

    void Init()
    {
#if defined(ESP8266)
        EEPROM.begin(1024);
#endif
    }

    TSettings();
    ~TSettings();
};

TSettings::TSettings()
{
}

TSettings::~TSettings()
{
}