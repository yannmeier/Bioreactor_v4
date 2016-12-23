#include <LiquidCrystal.h>
#include <SPI.h>
// initialize the library with the numbers of the interface pins

const int LCDD7 = A6;
const int LCDD6 = 12;
const int LCDD5 = 6;
const int LCDD4 = 8;
const int LCDE  = 9;
const int LCDRS = 10;

LiquidCrystal lcd(LCDRS,LCDE,LCDD4,LCDD5,LCDD6,LCDD7);

#define MAX_PARAM  52
#define MAX_CONFIG_PARAM 7             // Maximum number of configurable parameters in the config menu

#define PARAM_TEMP_LIQ             0   // temperature of the solution
#define PARAM_TEMP_PCB             1   // temperature of the heating plate
#define PARAM_WEIGHT               2   // in gr
#define PARAM_WEIGHT_FACTOR        15  // Weight calibration: conversion factor digital -> gr (weight=FACTOR*dig_unit)
#define PARAM_WEIGHT_OFFSET        16  // Weight calibration: digital offset value when bioreactor is empty
#define PARAM_STEPPER_SPEED        18  // motor speed, parameter S (!!!!!TO BE REPROGRAMMED IN RPM!!!!!!!)
#define PARAM_WEIGHT_STATUS        23  // current STATUS // BBBAAAAA AAAAAAAA : A = wait time in minutes, B = status
#define PARAM_STATUS               25  
#define PARAM_TEMP_TARGET          26  // target temperature of the liquid
#define PARAM_TEMP_MAX             27  // maximal temperature of the plate
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
#define MENU_SET_VALUE      4

#define OUT_BUF_SIZE    5


/************************************************
          Encoder position variables
*************************************************/
volatile unsigned int encoderMenuSelect  = MENU_SENSOR;  // a counter for the menu slection dial
volatile unsigned int encoderTempValue   = 0;  // a counter to store a temporary value before to set it
volatile unsigned int encoderLastValue   = 0;  // a counter to store a previous temporary value 
/************************************************
                Global Flags
*************************************************/
static boolean toBeRefreshed = true; //call the Refresh function on the next main loop call
static boolean rotating=false;       // debounce management
static boolean pushing  =true;       // debounce management
//ISR variables
boolean A_set = false;              
boolean B_set = false;
volatile boolean processIt=false;     // SPI buffer ready to be parsed ?
volatile boolean firstReturn=false;   // is first return in ending of SPI com from master?

/************************************************
              Parameter Utilities
*************************************************/

typedef struct{
  char* paramName = 0;
  int paramNameLength = 0;
  int paramNum = 0;    
} parameter;

parameter * configMenuParams [MAX_CONFIG_PARAM];

void printParam(int pos){
  if(pos>=0 && pos < MAX_CONFIG_PARAM){
    parameter * param = configMenuParams[pos];
    Serial.print(F("Param name:"));
    Serial.println(param->paramName);
    Serial.print(F("Param #: "));
    Serial.println(param->paramNum);  
  }else{
    Serial.println(F("Err: pos of config variable out of range"));  
  }
}

void addParam(int pos, char * id, int id_length, int number){
  if(pos>=0 && pos < MAX_CONFIG_PARAM){
    parameter * newParam = (parameter *) malloc(id_length+2*sizeof(int));
    newParam->paramName = id;
    newParam->paramNameLength = id_length;
    newParam->paramNum = number;
    configMenuParams[pos] = newParam;  
    printParam(pos);
  }else{
    Serial.println(F("Err: pos of config variable is out of range"));  
  }
}

/************************************************
         Arduino SPI Slave Functions
*************************************************/
//#define LCD_SELECT RXLED //pin SS (D8)
byte out_buf [OUT_BUF_SIZE];            // buffer for output to motherboard 
boolean writeToMaster=false;                // flag indicating if their is something to send to the motherboard
boolean is_start=false;                     // flag indicating if it's the beginning of the communication with the motherboard
byte buf [2*MAX_PARAM+2];     // buffer for input from motherboard
volatile byte pos;            // position of incoming byte in SPI buffer

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
  processIt = false;
  // now turn on interrupts
  SPI.attachInterrupt();
}

/**************************************************
                Data Parsing
***************************************************/
uint16_t param [MAX_PARAM];

//add checkDigit support here
void bufferParse(){
  byte buffSize= buf[0];
  byte checkDigit=buffSize;  
  for(int i=0;i<buffSize;i++){ //first check that the chkDgt is ok
    checkDigit^=buf[2*i+1];
    checkDigit^=buf[2*(i+1)];
  }
  if(checkDigit==buf[buffSize]){ //then load parameters
    for(int i=0;i<buffSize;i++){
       param[i]=((buf[2*i+1]<<8)&(0xFF00))+(buf[2*(i+1)]&(0x00FF));
       //    Serial.print(i);
       //    Serial.print(F(": "));
       //    Serial.println(param[i]);
    }
  }else Serial.println(F("wrong checkDigit"));

}

/************************************************
          SPI communication utilities
*************************************************/
/**
 * Set parameter on motherboard through SPI communication. 
 * int parameter : location of parameter in param list. Use defined parameters.
 * int value : value to be set for the given parameter. 
 */
void sendParameter(int parameter, int value){
  out_buf[0]=4;                                     // Start transmission with non_null byte (size of the message after)
  byte checkDigit=out_buf[0];
  out_buf[1]=(byte)parameter;                       // Number of parameter to set
  checkDigit^=out_buf[1];
  out_buf[2]=(byte)((value>>8)&(0x00FF));           // Value of parameter
  checkDigit^=out_buf[2];
  out_buf[3]=(byte)(value&(0x00FF));
  checkDigit^=out_buf[3];
  out_buf[4]=checkDigit;
  writeToMaster = true;                           // notify SPI interrupt that it can start sending bytes as soon as the transmission starts
  //Serial.println(F("Sent param")); 
}
/************************************************
       Main function to refresh the LCD
*************************************************/
void menuRefresh()
{  
  switch(encoderMenuSelect){
    case MENU_SELECTOR:
      displayMenuSelector();
      break;
    case MENU_SENSOR:
      displayMenuSensor();
      break;
    case MENU_CONFIG:
      displayMenuConfig();
      break;
    case MENU_CALIBRATION:
      displayMenuCalibr();
      break;
    default:
      return;
  }
  toBeRefreshed=0;
}

void valuesRefresh()
{
  switch(encoderMenuSelect){
    case MENU_SELECTOR:
      displayValueSelector();
      break;
    case MENU_SENSOR:
      displayValueSensor();
      break;
    case MENU_CONFIG:
      displayValueConfig();
      break;
    case MENU_CALIBRATION:
      displayValueCalibr();
      break;
    case MENU_SET_VALUE:
      configSetValue();
    default:
      return;
  }
}
/********************************************
       "Menu Display" Utilities Set
*********************************************/
void displayParamValue(int value, int space, int x, int y){
  lcd.setCursor(x, y);
  lcd.print(value);
  String value_string = String(value);
  for(int i = 0; i<(space-value_string.length()); i++){
    lcd.print(" ");  
  }    
}

void displayMenuSelector()
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

void displayValueSelector()
{     
 if(encoderTempValue!=encoderLastValue){
    lcd.setCursor(16,0);
    lcd.print((encoderTempValue%3)+1);
    lcd.setCursor(0,(encoderTempValue%3)+1);
    encoderLastValue=encoderTempValue;
  }
}


void displayMenuSensor()
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

void displayValueSensor()
{
  //display liquid temperature
  displayParamValue(param[PARAM_TEMP_LIQ], 7, 3, 0);
  //display motor speed in RPM
  displayParamValue(param[PARAM_STEPPER_SPEED], 6, 14, 0);
  //display weight
  displayParamValue(param[PARAM_WEIGHT], 7, 3, 2);
}

void displayMenuConfig()
{
  lcd.begin(20, 4);                             // Clear Screen
  
  lcd.setCursor(0,0);                           // print back button
  lcd.print(F("0)Bck"));
  
  for(int paramPos = 0; paramPos < MAX_CONFIG_PARAM; paramPos++){  // print all parameters
    lcd.setCursor(10*((paramPos+1)%2),(paramPos+1)/2);
    lcd.print(paramPos+1);
    lcd.print(")");
    lcd.print(configMenuParams[paramPos]->paramName);
    lcd.print(":");
  }
  
}


void displayValueConfig()
{     
  for(int paramNum = 0; paramNum < MAX_CONFIG_PARAM; paramNum++){  // print all parameters
    displayParamValue(param[configMenuParams[paramNum]->paramNum], 8-(configMenuParams[paramNum]->paramNameLength)-1, 10*((paramNum+1)%2)+configMenuParams[paramNum]->paramNameLength+3, (paramNum+1)/2);
  }
  lcd.setCursor(10*((encoderTempValue%8)%2),(encoderTempValue%8)/2);
}

void configSetValue()
{
  byte paramNum = (encoderTempValue%8)-1;
  byte paramValPos [2] = {10*((encoderTempValue%8)%2)+configMenuParams[paramNum]->paramNameLength+2, (encoderTempValue%8)/2}; // {char_num, line_num}
  int temp_value = param[configMenuParams[paramNum]->paramNum];
  lcd.setCursor(paramValPos[0], paramValPos[1]);
  if(paramNum >=0){
    while(encoderMenuSelect==MENU_SET_VALUE){
      if(encoderTempValue!=encoderLastValue){
        if(encoderTempValue>encoderLastValue){
          temp_value+=5;        
        } else {
          temp_value-=5;
        }
        displayParamValue(temp_value, 8-configMenuParams[paramNum]->paramNameLength-1, paramValPos[0]+1, paramValPos[1]);
        lcd.setCursor(paramValPos[0], paramValPos[1]);
        encoderLastValue = encoderTempValue;
      }  
    }
    sendParameter(configMenuParams[paramNum]->paramNum, temp_value);
  } 
}


void displayMenuCalibr() //all is to be implemented
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

void displayValueCalibr()
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
  
  // set configurable parameters
  delay(3000);                        // let Serial init 
  addParam(0, "RPM", 3, PARAM_STEPPER_SPEED);
  addParam(1, "Tp", 2, PARAM_TEMP_TARGET);
  addParam(2, "T+", 2, PARAM_TEMP_MAX);
  addParam(3, "W+", 2, PARAM_WEIGHT_MAX);
  addParam(4, "W-", 2, PARAM_WEIGHT_MIN);
  addParam(5, "Sd", 2, PARAM_SEDIMENTATION_TIME);
  addParam(6, "Md", 2, PARAM_STEPPER_SPEED);                          // unknown parameter number !!??
  
}

// main loop, work is done by interrupt service routines, this one only prints stuff
void loop() { 
  //refresh the menu if needed
  if(toBeRefreshed)
    menuRefresh();
  if (processIt)
    {
      bufferParse();
      valuesRefresh();
      pos = 0;
      processIt = false;
    }    
  lcd.blink();  
  delay(20);
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
      if((encoderTempValue%8)==0){
        encoderMenuSelect=MENU_SELECTOR;
      } else {
        encoderMenuSelect=MENU_SET_VALUE;  
      }
    } 
    else if (encoderMenuSelect==MENU_SET_VALUE)
      encoderMenuSelect=MENU_CONFIG;
      
    //calibration menu case
    else if(encoderMenuSelect==MENU_CALIBRATION){
      if((encoderTempValue%5)==0)
          encoderMenuSelect=MENU_SELECTOR;  
    }  
    pushing=false;     //debouncer  
    toBeRefreshed=1; //refresh flag

    //sendParameter(PARAM_STEPPER_SPEED, 30); //notify motherboard on button pressed
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
      firstReturn=true;
    if (c=='\n' && firstReturn==true)  
      processIt = true;
    else
      firstReturn=false; 
  }  // end of room available

  //////////////
  // Send answer
  //////////////

  if (!pos){                            // start sending out bytes when the first byte is received
    is_start = true;  
  }
  
  if (is_start && writeToMaster){     // send bytes if begining of communication and if their is something...
    SPDR = out_buf[pos];                // ...new to send to the master  
    if(pos+1>=OUT_BUF_SIZE){
      writeToMaster=false;
      is_start = false;
    }
  }
  else
    SPDR = 0;

  pos++;
}  // end of interrupt routine SPI_STC_vect

