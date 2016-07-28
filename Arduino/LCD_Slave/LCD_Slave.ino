#include <LiquidCrystal.h>
#include <SPI.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(10,9,8,6,12,A6);

#define MAX_PARAM  52

#define PARAM_TEMP_LIQ             0   // temperature of the solution
#define PARAM_TEMP_PCB             1   // temperature of the heating plate
#define PARAM_WEIGHT               2   // in gr
#define PARAM_WEIGHT_FACTOR        15  // Weight calibration: conversion factor digital -> gr (weight=FACTOR*dig_unit)
#define PARAM_WEIGHT_OFFSET        16  // Weight calibration: digital offset value when bioreactor is empty
#define PARAM_STEPPER_SPEED        18  // motor speed, parameter S (!!!!!TO BE REPROGRAMMED IN RPM!!!!!!!)
#define PARAM_WEIGHT_STATUS        23  // current STATUS // BBBAAAAA AAAAAAAA : A = wait time in minutes, B = status
#define PARAM_STATUS               25  
#define PARAM_TEMP_TARGET          26  // target temperature of the liquid
#define PARAM_TEMP_REG_TIME        28  //in [ms]
#define PARAM_WEIGHT_MIN           29    
#define PARAM_WEIGHT_MAX           30  
#define PARAM_SEDIMENTATION_TIME   35  // MINUTES to wait without rotation before emptying
#define PARAM_FILLED_TIME          36  // MINUTES to stay in the filled state


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

#define SPI_OUT_BUF_SIZE    3

//volatile unsigned int encoderParamSelect = 0;  // a counter for the configuration dial & Calibration dial ---> not used !!!!!
volatile unsigned int encoderMenuSelect  = MENU_SENSOR;  // a counter for the menu slection dial
volatile unsigned int encoderTempValue   = 0;  // a counter to store a temporary value before to set it
volatile unsigned int encoderLastValue   = 0;  // a counter to store a previous temporary value 
static boolean To_Be_Refreshed = true; // call the Refresh function on the next main loop call
static boolean rotating=false;         // debounce management
static boolean pushing  =true;         // debounce management
// interrupt service routine variables
boolean A_set = false;              
boolean B_set = false;

/************************************************
         Arduino SPI Slave Functions
*************************************************/
//#define LCD_SELECT RXLED //pin SS (D8)

byte out_buf [SPI_OUT_BUF_SIZE];                  // buffer for output to motherboard 
boolean write_to_master=false;                    // flag indicating if their is something to send to the motherboard
boolean is_start=false;                           // flag indicating if it's the beginning of the communication with the motherboard

byte buf [2*MAX_PARAM+2];     // buffer for input from motherboard
volatile byte pos;
volatile boolean process_it=false;
volatile boolean first_return=false;    

void SPI_slave_init()
{
  pinMode(SS,INPUT);     //switch the slave select pin to input mode
  digitalWrite(SS,HIGH); //turn on internal pull-up
  // turn on SPI in slave mode
  SPCR |= bit (SPE);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  // get ready for an interrupt 
  pos = 0;   // buffer empty
  process_it = false;
  // now turn on interrupts
  SPI.attachInterrupt();
}

/**************************************************
                Data Parsing
***************************************************/
char current_day;
char current_month;
char current_hour;
char current_minutes;
uint16_t param [MAX_PARAM];
uint32_t epoch;

void buffer_parser(){
 //  epoch=((buf[4]<<24) + (buf[5]<<16) + (buf[6]<<8) + buf[7]);
  for(int i=0;i<26;i++)
    param[i]=((buf[2*i]<<8)&(0xFF00))+(buf[2*i+1]&(0x00FF));
  
}

/************************************************
          SPI communication utilities
*************************************************/
void sendParameter(byte parameter, int value){
  out_buf[0]=parameter;
  out_buf[1]=(byte)((value>>8)&(0x00FF));
  out_buf[2]=(byte)(value&(0x00FF));
  write_to_master = true;     
}
/************************************************
       Main function to refresh the LCD
*************************************************/
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
    case MENU_SENSOR:
      Display_Value_Sensor();
      break;
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
       "Menu Display" Utilities Set
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

void Display_Value_Sensor()
{
  //display liquid temperature
  lcd.setCursor(3,0);
  lcd.print(param[PARAM_TEMP_LIQ]);
  //display motor speed in RPM
  lcd.setCursor(14,0);
  lcd.print(param[PARAM_STEPPER_SPEED]);      //--> motor speed to be moved in the first 26 parameters (eg S)
  //display weight
  lcd.setCursor(3,2);
  lcd.print(param[PARAM_WEIGHT]); 
  
/* if(encoderTempValue!=encoderLastValue){ //--> should remove the blinking in sensor display mode
    lcd.setCursor(10*((encoderTempValue%8)%2),(encoderTempValue%8)/2);
    encoderLastValue=encoderTempValue;
  }*/
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
    lcd.print(F("0)Back"));
    lcd.setCursor(10,0);
    lcd.print(F("1)Tare"));
    lcd.setCursor(0,1);
    lcd.print(F("2)Sp:"));
    lcd.setCursor(10,1);
    lcd.print(F("3)Of:"));
    lcd.setCursor(0,2);
    lcd.print(F("4)Calibration Proc"));  
    lcd.setCursor(0,3);
    lcd.print(F("gr:"));
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
  SPI_slave_init();
  lcd.begin(20, 4);
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() { 
  //refresh the menu if needed
  if(To_Be_Refreshed)
    Menu_Refresh();
  if (process_it)
    {
      buffer_parser();
      Values_Refresh();
      pos = 0;
      process_it = false;
    }    
  lcd.blink();  
  delay(10);
  rotating = true; 
  
 

}

/***************************************************************
                    Interrupt routines
****************************************************************/
// Interrupt on A changing state
void doEncoderA(){
//  delay(10);
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
//  delay(10);
  if ( rotating )
    delay(20);
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
       // TODO: add setting menu
    }  
    //calibration menu case
    else if(encoderMenuSelect==MENU_CALIBRATION){
      if((encoderTempValue%5)==0)
          encoderMenuSelect=MENU_SELECTOR;  
    }  
    pushing=false;     //debouncer  
    To_Be_Refreshed=1; //refresh flag

    sendParameter('a', 1); //notify motherboard on button pressed
  }
  else 
    pushing=true;
  delay(10);
}


// SPI interrupt routine
ISR (SPI_STC_vect)
{
  byte c = SPDR;  // grab byte from SPI Data Register
  
  /////////////////////
  // Read entering byte
  /////////////////////
  
  // add to buffer if room
  if (pos < sizeof(buf))
  {
     buf [pos] = c;                   //add byte to buffer
                                        // example: newline means time to process buffer
    if (c == '\n')
      first_return=true;
    if (c=='\n' && first_return==true)  
      process_it = true;
    else
      first_return=false; 
  }  // end of room available

  //////////////
  // Send answer
  //////////////

  if (!pos){                            // start sending out bytes when the first byte is received
    is_start = true;  
  }
  
  if (is_start && write_to_master){     // send bytes if begining of communication and if their is something...
    SPDR = out_buf[pos];                // ...new to send to the master  
    Serial.print("Sent a byte: ");
    Serial.println(out_buf[pos]);
    if(pos+1>=SPI_OUT_BUF_SIZE){
      write_to_master=false;
      is_start = false;
    }
  }
  else
    SPDR = 0;

  pos++;
}  // end of interrupt routine SPI_STC_vect

