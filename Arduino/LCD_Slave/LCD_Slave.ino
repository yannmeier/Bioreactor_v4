#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(10,9,8,6,12,A6);
 

#define ENCODER_CLOCKWISE      0
#define ENCODER_ANTI_CLOCKWISE 1
#define ENCODER_BUTTON         7
#define INT_CLOCKWISE          2
#define INT_ANTI_CLOCKWISE     3
#define INT_BUTTON             4  

#define MENU_SELECTOR       0
#define MENU_SENSOR         1
#define MENU_CONFIG         2
#define MENU_CALIBRATION    3

volatile unsigned int encoderParamSelect = 0;  // a counter for the configuration dial & Calibration dial 
volatile unsigned int encoderMenuSelect  = MENU_SENSOR;  // a counter for the menu slection dial
volatile unsigned int encoderTempValue   = 0;  // a counter to store a temporary value before to set it
volatile unsigned int encoderLastValue   = 0;  // a counter to store a previous temporary value 
static boolean To_Be_Refreshed = true;   // call the Refresh function on the next main loop call
static boolean rotating=false;        // debounce management
static boolean pushing  =true;        // debounce management
// interrupt service routine variables
boolean A_set = false;              
boolean B_set = false;


void Menu_Refresh()
{  
  switch(encoderMenuSelect){
    case MENU_SELECTOR:
      Display_Menu_Selector();
      break;
    case MENU_SENSOR:
      Display_Menu_Sensor();
      break;
    case MENU_CONFIG:
      Display_Menu_Config();
      break;
    case MENU_CALIBRATION:
      Display_Menu_Calibr();
      break;
    default:
      return;
  }
  To_Be_Refreshed=0;
}

void Values_Refresh()
{
  switch(encoderMenuSelect){
    case MENU_SELECTOR:
      Display_Value_Selector();
      break;
    /*case MENU_SENSOR:
      Display_Value_Sensor();
      break;*/
    case MENU_CONFIG:
      Display_Value_Config();
      break;
    case MENU_CALIBRATION:
      Display_Value_Calibr();
      break;
    default:
      return;
  }
}

/********************************************
       "Menu Display" Function Set
*********************************************/
void Display_Menu_Selector()
{
  lcd.begin(20, 4);
    lcd.setCursor(2,0);
    lcd.print(F("Menu Selector:"));
    lcd.setCursor(0,1);
    lcd.print(F("1)Sensors Display"));
    lcd.setCursor(0,2);
    lcd.print(F("2)Settings"));
    lcd.setCursor(0,3);
    lcd.print(F("3)Scale Calibration"));  
    lcd.setCursor(0,(encoderTempValue%3)+1);
}

void Display_Value_Selector()
{     
 if(encoderTempValue!=encoderLastValue){
    lcd.setCursor(16,0);
    lcd.print((encoderTempValue%3)+1);
    lcd.setCursor(0,(encoderTempValue%3)+1);
    encoderLastValue=encoderTempValue;
  }
}


void Display_Menu_Sensor()
{
  lcd.begin(20, 4);
    lcd.setCursor(0,0);
    lcd.print(F("Tp:"));
    lcd.setCursor(10,0);
    lcd.print(F("RPM:"));
    lcd.setCursor(0,1);
    lcd.print(F("Pp:"));
    lcd.setCursor(10,1);
    lcd.print(F("Sd:"));
    lcd.setCursor(0,2);
    lcd.print(F("gr:"));
}


void Display_Menu_Config()
{
  lcd.begin(20, 4);
    lcd.setCursor(0,0);
    lcd.print(F("0)Bck"));
    lcd.setCursor(10,0);
    lcd.print(F("1)RPM"));
    lcd.setCursor(0,1);
    lcd.print(F("2)Tp:"));
    lcd.setCursor(10,1);
    lcd.print(F("3)T+:"));
    lcd.setCursor(0,2);
    lcd.print(F("4)W+:"));
    lcd.setCursor(10,2);
    lcd.print(F("5)W-:"));
    lcd.setCursor(0,3);
    lcd.print(F("6)Sd:"));
    lcd.setCursor(10,3);
    lcd.print(F("7)Md:"));
    lcd.setCursor(10*((encoderTempValue%8)%2),(encoderTempValue%8)/2);
}

void Display_Value_Config()
{     
 if(encoderTempValue!=encoderLastValue){
    lcd.setCursor(10*((encoderTempValue%8)%2),(encoderTempValue%8)/2);
    encoderLastValue=encoderTempValue;
  }
}


void Display_Menu_Calibr() //all is to be implemented
{
  lcd.begin(20, 4);
    lcd.setCursor(0,0);
    lcd.print("0)Back");
    lcd.setCursor(10,0);
    lcd.print("1)Tare");
    lcd.setCursor(0,1);
    lcd.print("2)Sp:");
    lcd.setCursor(10,1);
    lcd.print("3)Of:");
    lcd.setCursor(0,2);
    lcd.print("4)Calibration Proc");  
    lcd.setCursor(0,3);
    lcd.print("gr:");
    lcd.setCursor(10*((encoderTempValue%5)%2),(encoderTempValue%5)/2);
}

void Display_Value_Calibr()
{     
 if(encoderTempValue!=encoderLastValue){
    lcd.setCursor(10*((encoderTempValue%5)%2),(encoderTempValue%5)/2);
    encoderLastValue=encoderTempValue;
  }
}

/**************************************************
                Setup and Main Loop
***************************************************/
void setup() {  
  pinMode(ENCODER_CLOCKWISE, INPUT); 
  pinMode(ENCODER_ANTI_CLOCKWISE, INPUT); 
  pinMode(ENCODER_BUTTON, INPUT);
  // turn on iinternal pullup resistors
  digitalWrite(ENCODER_CLOCKWISE, HIGH);
  digitalWrite(ENCODER_ANTI_CLOCKWISE, HIGH);
  digitalWrite(ENCODER_BUTTON, HIGH);
  // attach interrupt routines
  attachInterrupt(INT_CLOCKWISE, doEncoderA, CHANGE);
  attachInterrupt(INT_ANTI_CLOCKWISE, doEncoderB, CHANGE);
  attachInterrupt(INT_BUTTON, doEncoderButton, CHANGE);
  //Serial communication start
  Serial.begin(9600);  
  lcd.begin(20, 4);
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() { 
  //refresh the menu if needed
  if(To_Be_Refreshed)
    Menu_Refresh();
  Values_Refresh();
  lcd.blink();  
  delay(10);
  rotating = true; 
}

/***************************************************************
                    Interrupt routines
****************************************************************/
// Interrupt on A changing state
void doEncoderA(){
  delay(1);
  if ( rotating ) 
    delay(20);//debounce
    if( digitalRead(ENCODER_CLOCKWISE) != A_set ) {  // debounce once more
      A_set = !A_set;
      if ( A_set && !B_set ) 
        encoderTempValue++;
    }
    rotating = false;  // no more debouncing until loop() hits again
}

// Interrupt on B changing state, same as A above
void doEncoderB(){
  delay(20);
  if ( rotating )
    delay(10);
    if( digitalRead(ENCODER_ANTI_CLOCKWISE) != B_set ) {
      B_set = !B_set;
      //  adjust counter - 1 if B leads A
      if( B_set && !A_set ) 
        encoderTempValue--;

    rotating = false;
  }
}

// Interrupt on B changing state, same as A above
void doEncoderButton(){
  if (pushing){
    //main menu case
    if(encoderMenuSelect==MENU_SELECTOR )  
      encoderMenuSelect=(encoderTempValue%3)+1;
    //sensor display case
    else if(encoderMenuSelect==MENU_SENSOR)
      encoderMenuSelect=MENU_SELECTOR;
    //setting parameters case
    else if(encoderMenuSelect==MENU_CONFIG){
      if((encoderTempValue%8)==0)
        encoderMenuSelect=MENU_SELECTOR;
    }  
    //calibration menu case
    else if(encoderMenuSelect==MENU_CALIBRATION){
      if((encoderTempValue%5)==0)
          encoderMenuSelect=MENU_SELECTOR;
      
    }
       

    pushing=false;     //debouncer  
    To_Be_Refreshed=1; //refresh flag
  }
  else 
    pushing=true;
  delay(10);
}


