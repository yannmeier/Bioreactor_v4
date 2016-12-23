#include <LiquidCrystal.h>
#include <SPI.h>
#include "LCD_Slave.h"
// initialize the lib with the nbr of the interface pins
LiquidCrystal lcd(LCDRS,LCDE,LCDD4,LCDD5,LCDD6,LCDD7);
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
    Serial.print(F("ParamName:"));
    Serial.println(param->paramName);
    Serial.print(F("Param#:"));
    Serial.println(param->paramNum);  
  }else{
    Serial.println(F("Err: pos of config variable out of range"));  
  }
}

void addParam(int pos, char * id, int idLength, int number){
  if(pos>=0 && pos < MAX_CONFIG_PARAM){
    parameter * newParam = (parameter *) malloc(idLength+2*sizeof(int));
    newParam->paramName = id;
    newParam->paramNameLength = idLength;
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
byte outBuf [OUT_BUF_SIZE];     // output SPI buffer
boolean writeToMaster=false;   // flag indicating if their is something to send 
boolean isStart=false;         // flag indicating if beginning of the COM 
byte buf [2*MAX_PARAM+1];      // buffer for input from motherboard : maxparams + XOR
volatile byte pos;            // position of incoming byte in SPI buffer

void slaveInit()
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
       //Serial.print(F("Changed Local Param"));
    }
  }else Serial.println(F("wrong XOR"));
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
  outBuf[0]=4;                                     // Start transmission with non_null byte (size of the message after)
  byte checkDigit=outBuf[0];
  outBuf[1]=(byte)parameter;                       // Number of parameter to set
  checkDigit^=outBuf[1];
  outBuf[2]=(byte)((value>>8)&(0x00FF));           // Value of parameter
  checkDigit^=outBuf[2];
  outBuf[3]=(byte)(value&(0x00FF));
  checkDigit^=outBuf[3];
  outBuf[4]=checkDigit;
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
  toBeRefreshed=false;
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
/************************************************************
            "Values Display" Utilities Set
*************************************************************/
void displayParamValue(int value, int space, int x, int y){
  lcd.setCursor(x, y);
  lcd.print(value);
  String value_string = String(value);
  for(int i = 0; i<(space-value_string.length()); i++){
    lcd.print(" ");  
  }    
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


void displayValueSensor()
{
  displayParamValue(param[PARAM_TEMP_LIQ], 7, 3, 0);       //display liquid temperature
  displayParamValue(param[PARAM_STEPPER_SPEED], 6, 14, 0); //display motor speed in RPM
  displayParamValue(param[PARAM_WEIGHT], 7, 3, 2);         //display weight
}


void displayValueConfig()
{     
  for(int paramNum = 0; paramNum < MAX_CONFIG_PARAM; paramNum++){  // print all parameters
    displayParamValue(param[configMenuParams[paramNum]->paramNum], 
                       8-(configMenuParams[paramNum]->paramNameLength)-1, 
                       10*((paramNum+1)%2)+configMenuParams[paramNum]->paramNameLength+3, 
                       (paramNum+1)/2);
  }
  lcd.setCursor(10*((encoderTempValue%8)%2),(encoderTempValue%8)/2);
}


void displayValueCalibr()
{     
 if(encoderTempValue!=encoderLastValue){
    lcd.setCursor(10*((encoderTempValue%5)%2),(encoderTempValue%5)/2);
    encoderLastValue=encoderTempValue;
  }
}

/************************************************************
            "Menu Display" Utilities Set
*************************************************************/
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


void configSetValue()
{
  byte paramNum = (encoderTempValue%8)-1;
  byte paramValPos [2] = {10*((encoderTempValue%8)%2)+configMenuParams[paramNum]->paramNameLength+2, (encoderTempValue%8)/2}; // {char_num, line_num}
  int tempVal = param[configMenuParams[paramNum]->paramNum];
  lcd.setCursor(paramValPos[0], paramValPos[1]);
  if(paramNum >=0){
    while(encoderMenuSelect==MENU_SET_VALUE){
      if(encoderTempValue!=encoderLastValue){
        if(encoderTempValue>encoderLastValue){
          tempVal+=5;        
        } else {
          tempVal-=5;
        }
        displayParamValue(tempVal, 8-configMenuParams[paramNum]->paramNameLength-1, paramValPos[0]+1, paramValPos[1]);
        lcd.setCursor(paramValPos[0], paramValPos[1]);
        encoderLastValue = encoderTempValue;
      }  
    }
    sendParameter(configMenuParams[paramNum]->paramNum, tempVal);
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
  slaveInit();
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

// main loop, work is done by ISRs, this one only prints stuff
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
    //main menu 
    if(encoderMenuSelect==MENU_SELECTOR) encoderMenuSelect=(encoderTempValue%3)+1;  //sensor display case
    else if(encoderMenuSelect==MENU_SENSOR) encoderMenuSelect=MENU_SELECTOR;        //setting parameters case
    else if(encoderMenuSelect==MENU_CONFIG){
      if((encoderTempValue%8)==0){
        encoderMenuSelect=MENU_SELECTOR;
      } else {
        encoderMenuSelect=MENU_SET_VALUE;  
      }
    } 
    else if (encoderMenuSelect==MENU_SET_VALUE)
      encoderMenuSelect=MENU_CONFIG;
    //calibration menu 
    else if(encoderMenuSelect==MENU_CALIBRATION){
      if((encoderTempValue%5)==0)
          encoderMenuSelect=MENU_SELECTOR;  
    }  
    pushing=false;   //debouncer  
    toBeRefreshed=true; //refresh flag
    //sendParameter(PARAM_STEPPER_SPEED, 30); //notify motherboard on button pressed
  }
  else 
    pushing=true;
  delay(10);
}

/***************************************************************
                      SPI     ISR
****************************************************************/
ISR (SPI_STC_vect)
{
  byte c = SPDR;  // Read entering byte on SPI Data Register  
  byte buffSize=0;
  
  //receive Data
  if (pos==0){
    isStart = true;  // start sending out bytes when the first byte is received
    buffSize=c;      // first message holds the size incoming msg
  }else if (pos <= buffSize){ // '=<'  not '<'because we have a XOR at the end
    buf[pos-1] = c;       //read until message length is reached                  
    if(pos==buffSize) processIt=true; //message captured
  }
  
  //send Data
  if (isStart && writeToMaster){   // send bytes if begining of COM AND if their is something
    SPDR = outBuf[pos];            // ...new to send to the master  
    if(pos+1>=OUT_BUF_SIZE){
      writeToMaster=false;
      isStart = false;
    }
  }else SPDR = 0;
  //increment counter
  pos++;
}

