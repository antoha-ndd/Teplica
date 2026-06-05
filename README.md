# Teplica

Прошивка автоматизации теплицы на ESP32: управление тремя моторизованными проёмами (2 окна + дверь) по температуре, кнопкам и веб-интерфейсу.

## Стек

- PlatformIO + Arduino (Espressif32)
- Библиотека компонентов [TApplication](https://github.com/antoha-ndd/TApplication)
- [Gyver Settings](https://github.com/GyverLibs/Settings) — веб-UI
- BMP180 — температура
- SSD1306 — OLED-дисплей

## Структура проекта

```
src/
  main.cpp          — setup/loop
  AppSettings.cpp   — Init(), создание объектов
  events.cpp        — обработчики кнопок и таймеров
  webui.cpp         — веб-интерфейс (Gyver Settings)
  settings.cpp      — Load/Save настроек в NVS
  var.cpp           — глобальные переменные
include/
  config.h          — константы (MOTOR_COUNT, гистерезис)
  var.h             — объявления глобальных переменных
  events.h          — объявления обработчиков
  webui.h           — веб-интерфейс
  settings.h        — API настроек (NVS)
  hardware_pins.h   — GPIO
lib/TApplication/   — фреймворк компонентов
```

## GPIO

| Назначение | Open | Close |
|------------|------|-------|
| Окно ближнее | GPIO 14 | GPIO 27 |
| Окно дальнее | GPIO 12 | GPIO 13 |
| Дверь дальняя | GPIO 2 | GPIO 26 |

Кнопки: Open 5/4/18, Close 19/16/17.

GPIO 2 — strapping pin ESP32; при проблемах загрузки переназначить в `include/hardware_pins.h`.

## Сборка и загрузка

```bash
python -m platformio run
python -m platformio run -t upload
```

### OTA-пароль (локально)

Скопируйте `platformio_private.ini.example` в `platformio_private.ini` и укажите пароль OTA. Файл в `.gitignore`.

Пароль также можно записать в NVS ключом `ota_pass` (namespace `config`).

## WiFi

- **STA:** настройки `wifi_ssid` / `wifi_pass` в NVS (веб-вкладка WI-FI → «Применить»)
- **Fallback AP:** `AutoConnectAP`, пароль `teplica-<CHIP_ID>`
- **Pairing:** двойной клик на кнопке Open 1 — сброс WiFi и перезагрузка

## Веб-интерфейс

Gyver Settings на порту 80 (`http://<IP>/` или `http://teplica.local/`). Вложенные меню: три проёма (пороги, автооткрытие/закрытие, ручное управление), WiFi, перезагрузка. Автообновление каждые 500 мс.

## Автоматизация по температуре

- **Открытие:** температура > порога открытия + 0.5°C
- **Закрытие:** температура < порога закрытия − 0.5°C
- Порог открытия должен быть выше порога закрытия; при сохранении некорректных значений они нормализуются автоматически

## OTA

Порт 8266, hostname `ESP_Board`. Прогресс отображается на OLED.
