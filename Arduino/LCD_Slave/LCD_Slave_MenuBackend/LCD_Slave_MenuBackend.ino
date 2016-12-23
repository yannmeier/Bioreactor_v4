/***************************************************************
                    Included libraries 
****************************************************************/

//#include <MenuBackend.h> // Generic menu management lib
#include <LiquidCrystal.h> // LCD display lib
#include <SPI.h> // SPI communication lib

/***************************************************************
                    Constants & Definitions
****************************************************************/

#define LCDD7  A6
#define LCDD6  12
#define LCDD5  6
#define LCDD4  8
#define LCDE   9
#define LCDRS  10

LiquidCrystal lcd(LCDRS,LCDE,LCDD4,LCDD5,LCDD6,LCDD7);

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

#define SPI_OUT_BUF_SIZE    4

/***************************************************************
                    Global Variables
****************************************************************/

///////////////////////
// Menu system
///////////////////////

typedef struct{
   int value;
   char* item_name;   
} menuItem;

menuItem param [MAX_PARAM];

///////////////////////
// SPI and data parsing
///////////////////////

byte out_buf [SPI_OUT_BUF_SIZE];                  // buffer for output to motherboard 
boolean write_to_master=false;                    // flag indicating if their is something to send to the motherboard
boolean is_start=false;                           // flag indicating if it's the beginning of the communication with the motherboard

byte buf [2*MAX_PARAM+2];     // buffer for input from motherboard
volatile byte pos;
uint16_t param [MAX_PARAM];

///////////////////////
// Encoder Values
///////////////////////

volatile unsigned int encoderTempValue   = 0;  // a counter to store a temporary value before to set it
volatile unsigned int encoderLastValue   = 0;  // a counter to store a previous temporary value 

///////////////////////
// flags
///////////////////////

volatile boolean process_it=false;
volatile boolean first_return=false;    
static boolean rotating=false;         // debounce management
static boolean pushing  =true;         // debounce management
// interrupt service routine variables
boolean A_set = false;              
boolean B_set = false;


/***************************************************************
                    SPI related routines
****************************************************************/

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

void buffer_parser(){
 //  epoch=((buf[4]<<24) + (buf[5]<<16) + (buf[6]<<8) + buf[7]);
  for(int i=0;i<26;i++)
    param[i]=((buf[2*i]<<8)&(0xFF00))+(buf[2*i+1]&(0x00FF));  
}

/**
 * Set parameter on motherboard through SPI communication. 
 * int parameter : location of parameter in param list. Use defined parameters.
 * int value : value to be set for the given parameter. 
 */
void sendParameter(int parameter, int value){
  out_buf[0]=1;                                     // Start transmission with non_null byte
  out_buf[1]=(byte)parameter;                       // Number of parameter to set
  out_buf[2]=(byte)((value>>8)&(0x00FF));           // Value of parameter
  out_buf[3]=(byte)(value&(0x00FF));
  write_to_master = true;                           // notify SPI interrupt that it can start sending bytes as soon as the transmission starts
}

/***************************************************************
                    LCD Display routines
****************************************************************/

void LCD_println(int line_num, char* text){
  lcd.setCursor(0, line_num);
  lcd.print(text);  
}

void LCD_printItem(){}
/***************************************************************
                    Setup & Loop routines
****************************************************************/

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
  LCD_println(0, "Test");
}

void loop() {
  // put your main code here, to run repeatedly:

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
    pushing=false;     //debouncer  
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

