#include <espwifi.h>
#include "Objects.h"
#include "bmp180.h"
#include "ObjectTimer.h"
#include "ssd1306.h"
#include "tpcf8575.h"

TApplication *App;
TBMP180 *bmp;
TTimer *Timer1;
TSSD1306 *LCD;
TPCF8575 *PCF;

TPCF8575_Button *Btn1, *Btn2, *Btn3, *Btn4, *Btn5, *Btn6;
TPCF8575_OutputDevice *Od1, *Od2, *Od3, *Od4, *Od5, *Od6;
// open 10 13 12
// close 8 9 11

void Timer1_Timeout(TTimer *Timer){

    App->PrintLn( String( bmp->Temperature( true ) ) );
    
    LCD->clearDisplay();
    LCD->setCursor(20,5);
    LCD->setTextSize(3);
    LCD->print( String( bmp->Temperature() ).c_str() );
    LCD->display();

};

void Btn1_OnClick(TPCF8575_Button *Button){
    
    App->PrintLn( "1 Click"  );
    LCD->clearDisplay();
    LCD->setCursor(1,5);
    LCD->setTextSize(2);

    LCD->print(  String( Button->GetPin() ) + " Click" );
    LCD->display();

};


void Btn1_OnPress(TPCF8575_Button *Button){

    App->PrintLn( "1 Press"  );

    LCD->clearDisplay();
    LCD->setCursor(1,5);
    LCD->setTextSize(2);

    LCD->print( " Press" );
    LCD->display();

};

void Btn1_OnRelease(TPCF8575_Button *Button){

    App->PrintLn( " Release"  );

    LCD->clearDisplay();
    LCD->setCursor(1,5);
    LCD->setTextSize(2);

    LCD->print( " Release" );
    LCD->display();


};

