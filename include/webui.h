#pragma once

namespace sets
{
    class Builder;
    class Updater;
}

class SettingsGyver;

extern SettingsGyver *ui;

void InitWebUi();
void buildUi(sets::Builder &b);
void updateUi(sets::Updater &upd);
