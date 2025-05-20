#pragma once
#include "simpledevice.h"
#include <Wire.h>

class TI2CComMaster : public TControl
{
public:
    TI2CComMaster() : TControl(NULL)
    {
        Wire.begin();
    }

    void setPin(int address, byte pin, byte value)
    {
        Serial.println("SEND " + String(address)+ " " + String(pin) + " " + String(value));
        Wire.beginTransmission(address);
        Wire.write(pin);   // Отправляем номер пина
        Wire.write(0);     // Код действия: 0 - запись
        Wire.write(value); // Значение (HIGH/LOW)
        Wire.endTransmission();
    }

    // Функция чтения состояния пина
    byte readPin(int address, byte pin)
    {
        Wire.beginTransmission(address);
        Wire.write(pin); // Отправляем номер пина
        Wire.write(1);   // Код действия: 1 - чтение
        Wire.endTransmission();

        Wire.requestFrom(address, 1); // Запрашиваем 1 байт
        return Wire.available() ? Wire.read() : 0xFF;
    }
};

class TI2CComSlave : public TControl
{
private:
    int Address{0};

public:
    bool Active{false};
    TI2CComSlave(int Addr) : TControl(NULL)
    {
        Address = Addr;
        Wire.begin(Address);
    }

    void Idle()
    {
        if(!Active) return;
        if (Wire.available() >= 3)
        {
            byte pin = Wire.read();
            byte action = Wire.read();
            byte value = Wire.read();

            if (action == 0)
            { // Команда записи
                pinMode(pin, OUTPUT);
                digitalWrite(pin, value);
            }
            else if (action == 1)
            { // Команда чтения
                pinMode(pin, INPUT_PULLUP);
                value   = digitalRead(pin);
                Wire.write(value);
            }
        }
    }
};