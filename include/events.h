#pragma once

class TButton;
class TTimer;

void OnOTAProgress(unsigned int Progress, unsigned int Total);

void BtnOpen0_Click(TButton *Button);
void BtnOpen1_Click(TButton *Button);
void BtnOpen2_Click(TButton *Button);
void BtnClose0_Click(TButton *Button);
void BtnClose1_Click(TButton *Button);
void BtnClose2_Click(TButton *Button);
void BtnPairing_DblClick(TButton *Button);

void Timer1_Timeout(TTimer *Timer);
void Timer2_Timeout(TTimer *Timer);
