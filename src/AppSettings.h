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

TPCF8575_Button *Btn1;

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
    LCD->setCursor(20,5);
    LCD->setTextSize(3);
    LCD->print( "1 Click" );
    LCD->display();

};


void Btn1_OnPress(TPCF8575_Button *Button){

    App->PrintLn( "1 Press"  );

    LCD->clearDisplay();
    LCD->setCursor(20,5);
    LCD->setTextSize(3);
    LCD->print( "1 Press" );
    LCD->display();

};

void Btn1_OnRelease(TPCF8575_Button *Button){

    App->PrintLn( "1 Release"  );

    LCD->clearDisplay();
    LCD->setCursor(20,5);
    LCD->setTextSize(3);
    LCD->print( "1 Release" );
    LCD->display();


};

