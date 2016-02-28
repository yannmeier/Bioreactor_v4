/**************
 * LIBRAIRIES
 **************/

//MultiThread
#include <NilRTOS.h>

//Lib to access memory with SPI
#include <SPI.h>

// Library that allows to start the watch dog allowing automatic reboot in case of crash
// The lowest priority thread should take care of the watch dog
#include <avr/wdt.h>

// http://www.arduino.cc/playground/Code/Time
// We need to deal with EPOCH
#include <Time.h>



/******************
 * DEFINE CARD TYPE
 ******************/

// THIS SHOULD BE THE ONLY PARAMETER THAT CHANGES !!!!

//#define TYPE_ETHERNET_GENERAL 1   // card that allows to control the pH, motor, ...
#define TYPE_ZIGBEE_GENERAL     1   // card to control the basic functions: pH, motor, temperature
//#define TYPE_ZIGBEE_GAS      1  // card to control the gas
//#define TYPE_PRECISE_PID    1

//#define TYPE_OLD_PIN_CONFIG     1   //define this if you use the card with the old pin configuration
//(old=until integratedBertha v1.0)

/******************
 * OPERATION MODE
 ******************/
//if you choose the calibration mode, you have to select a GENERAL card type first! 
//(not the GAS card, even when you calibrate the anemometer)
//#define MODE_CALIBRATE    1 //In this mode, you start the interactive calibration process.

/********************
 * PIN&ADRESS MAPPING
 *********************/
#ifdef TYPE_ETHERNET_GENERAL
#define PWM1    6//D6 OC4D
#define PWM2    8//D8 PCINT4
#define PWM3    9//D9 OC4B, OC1A, PCINT5
#define PWM4    5//D5 OC4A  
#define PWM5    11//D11 OC0A, OC1C, PCINT7
#define IO1     21//A3
#define IO2     20//A2
#define IO3     19//A1
#define IO4     22//A4
#define IO5     18//A0
#else 
#define PWM1    6//D6 OC4D
#define PWM3    9//D9 OC4B, OC1A, PCINT5
#define PWM4    5//D5 OC4A  
#define PWM5    11//D11 OC0A, OC1C, PCINT7
#define IO1     21//A3
#define IO2     20//A2
#define IO3     19//A1
#define IO4     22//A4
#ifdef TYPE_OLD_PIN_CONFIG
#define PWM2    8//D8 A8 PCINT4
#define IO5     18//D18 A0
#else
#define PWM2    10//D10 A10
#define IO5     8 //D8 A8
#endif
#endif

//I2C addresses 
#define I2C_FLUX          106//B01101000
#define I2C_PH            104//B01101000


/**************************************
 * ACTIVE THREAD DEPENDING CARD TYPE
 **************************************/

#ifdef TYPE_ZIGBEE_GENERAL
#define MODEL_ZIGBEE       1
//#define PH_CTRL_OLD        1
#define PH_CTRL_I2C        1
//#define PH_STRUCT          1
#define PH_TABLE           1
#define PH_CTRL            1
  #define TAP_BASE           PWM4
  #define TAP_ACID           IO4
#define THR_STEPPER        1
#define STEPPER            {IO5,PWM5}     
#define FOOD_CTRL          1   // food control use direct relay connected to one output using relay board
#define FOOD_IN            PWM1
#define FOOD_OUT           IO1
#define WEIGHT             11 //integrated weight sensor
#define TEMPERATURE_CTRL   1
#define TEMP_LIQ           23
#ifdef TYPE_OLD_PIN_CONFIG
  #define TEMP_PLATE       10 //D10 A10
#else
  #define TEMP_PLATE       18 //D18 A0
#endif
#define TEMP_PID           7
#define THR_LINEAR_LOGS    1
#define THR_MONITORING     1  // starts the blinking led and the watch dog counter 
#define MONITORING_LED     13
#define MASTER_PIN         IO2
#define MASTER_PWM         PWM2
#endif

#ifdef TYPE_ZIGBEE_GAS
#define MODEL_ZIGBEE       1
#define GAS_CTRL           1
#define THR_LINEAR_LOGS 	1
#define THR_MONITORING     1  // starts the blinking led and the watch dog counter
#define MONITORING_LED     13
//#define MASTER_PIN         IO4
//#define MASTER_PWM         PWM4
#endif

#ifdef TYPE_PRECISE_PID
  #define TEMPERATURE_CTRL   1
  #define TEMP_LIQ           23 //D23 A5
  #define TEMP_PLATE         18 //D18 A0
  #define TEMP_SAMPLE        20 //D20 A2
  #define TEMP_PID_HOT       7
  #define TEMP_PID_COLD      8
  #define THR_MONITORING     1  // starts the blinking led and the watch dog counter 
  #define MONITORING_LED     13
#endif

#ifdef TYPE_ETHERNET_GENERAL
#define PH_CTRL           1
#define TAP_BASE          PWM4
#define TAP_ACID          IO4
#define MODEL_ETHERNET    1
#define FOOD_CTRL         1
#define I2C_RELAY_FOOD    32 //B00100000
//#define I2C_RELAY_TAP     36 //B00100100
#define WEIGHT            1 //Analog port 1
#define TEMPERATURE_CTRL  1
#define TEMP_LIQ          IO1  
#define TEMP_PLATE        IO2    
//#define TEMP_PID          PWM4 CONFLICT WITH PH!!!
#define TEMP_PID          PWM3
#define THR_LINEAR_LOGS   1
#define MONITORING_LED    13
#define THR_MONITORING    1
#endif

#ifdef MODE_CALIBRATE
#define GAS_CTRL           1 
//undefine unused threads during calibration
#undef PH_CTRL 
#undef TAP_ACID
#undef TAP_BASE
#undef THR_STEPPER
#undef TEMPERATURE_CTRL
#endif

/***********************
 * SERIAL, LOGGER AND DEBUGGER
 ************************/

// #define EEPROM_DUMP   1   // Gives the menu allowing to dump the EEPROM

#define THR_SERIAL        1

#ifdef MODEL_ZIGBEE
#define THR_ZIGBEE      1 // communication process on Serial1
#endif

// NOT USED ANYMORE - USE Qualifier instead (directly from the menu)
// #define CARD_ID1 0xFE   // card ID should be stored as a parameter as am int !!!!
// #define CARD_ID2 0xAC


//Define here if the LCD screen is used or not
//#define I2C_LCD B00100111
//WIRE_LCD_16_2 B00100111
//WIRE_LCD_20_4 B00100110

/*******************************
 * THREADS AND PARAMETERS PRESENT IN EACH CARD 
 *******************************/

#ifdef THR_LINEAR_LOGS
#define LOG_INTERVAL          10  // define the interval in seconds between storing the log
// #define DEBUG_LOGS          1
#endif


#ifdef MODEL_ETHERNET
//#define THR_ETHERNET        1
// #define DEBUG_ETHERNET      1
#endif


/**********************
 * NETWORK PARAMETERS
 * // Enter a MAC address and IP address for the Arduino controller below.
 * // The IP address is reserved on the routher for this Arduino controller.
 * // CAUTION
 * // Each different boards should have a different IP in the range 172.17.0.100 - 172.17.0.200
 * // and a different MAC address
 ***********************/
//#define IP {172, 17, 0 , 103}
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAB}
//#define IP {172, 17, 0 , 107}
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAA}
//#define IP {172, 17, 0 , 105}
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}
//#define IP {172, 17, 0 ,101}                          //stepper
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}      //stepper
//#define IP {172, 17, 0 ,103}                             
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE}        
#define IP {172, 17, 0 ,104}                          //bertha 104
#define MAC {0xDE, 0xAD, 0xBE, 0xEF, CARD_ID1, CARD_ID2}      //bertha 104
//#define IP {10, 0, 0 ,105}                          //pH
//#define MAC {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}      //pH


/*******************************
 * CARD DEFINITION (HARD CODED)
 *******************************/
#ifdef STEPPER
  #define PARAM_STEPPER_SPEED       37   // motor speed
#endif

#ifdef     TEMPERATURE_CTRL
#define PARAM_TEMP_LIQ             0   // temperature of the solution
#define PARAM_TEMP_PLATE           1   // temperature of the heating plate
#define PARAM_TARGET_LIQUID_TEMP   26  // target temperature of the liquid
#define PARAM_TEMP_MAX             27  // maximal temperature of the plate
#ifdef TEMP_SAMPLE
  #define PARAM_TEMP_SAMPLE         2
#endif
#if defined (TEMP_PID) || defined (TEMP_PID_COLD)
  #ifdef TYPE_PRECISE_PID
  #define PARAM_AMBIENT_TEMP 32  //to autoswitch between modes
  #define PARAM_PID_STATUS  31   // 0 for heating - 1 for cooling
  #endif
//for the regulation of temperature values btw 10 and 45 [s] are commun
#define PARAM_HEATING_REGULATION_TIME_WINDOW  28 //in [ms]
#define PARAM_MIN_TEMPERATURE                 29 // not used but could be used for safety
#define PARAM_MAX_TEMPERATURE                 30 // not used but could be used for safety
#endif
#endif


//*************************************

#ifdef FOOD_CTRL       
#define PARAM_WEIGHT_FACTOR          15  // Weight calibration: conversion factor between digital unit and milligrams (weight=FACTOR*dig_unit)
#define PARAM_WEIGHT_OFFSET          16  // Weight calibration: digital offset value when bioreactor is empty

#define PARAM_WEIGHT                 2 // weight in g of the content in the bioreactor
#define PARAM_WEIGHT_MIN             31   // minimal weight  
#define PARAM_WEIGHT_MAX             32   // maximal weight
//hard coded safety value, TO BE CHANGED ONCE THE SENSOR IS CALIBRATED and conversion performed automatically !!!!!!!!!
#define PARAM_MIN_ABSOLUTE_WEIGHT    33
#define PARAM_MAX_ABSOLUTE_WEIGHT    34

#define PARAM_SEDIMENTATION_TIME     35   // number of MINUTES to wait without rotation before starting emptying
#define PARAM_MIN_FILLED_TIME        36   // minimal time in MINUTES to stay in the filled status
#define PARAM_WEIGHT_STATUS          51   // current STATUS // BBBAAAAA AAAAAAAA : A = wait time in minutes, B = status
#endif

//*************************************

#ifdef    PH_CTRL
#define PARAM_PH            3    // current pH
#define PARAM_TARGET_PH     38   // desired pH
#define PARAM_PH_FACTOR_A   39
#define PARAM_PH_FACTOR_B   40
#define PARAM_PH_STATE      41  // 0: Pause 1 : normal acquisition, 2 : purge of pipes,  4: calibration pH=4, 7: calibration pH=7, 10: calibration pH=10
#define PARAM_REF_PH4		12
#define PARAM_REF_PH7		13	
#define PARAM_REF_PH10		14	

//not parameters, hard coded values, set the minimal delay between pH adjustements to 10 seconds
#define PARAM_PH_ADJUST_DELAY      42    //delay between acid or base supplies
#define PARAM_PH_OPENING_TIME      43    //1sec TAP opening when adjusting
#define PARAM_PH_TOLERANCE         44    //correspond to a pH variation of 0.1
#endif

//*************************************

#ifdef     GAS_CTRL
//Calibration
#define PARAM_ANEMO_OFFSET1 17  // anemometer calibration: offset of the digital value (digital value when no gas is flowing)
#define PARAM_ANEMO_OFFSET2 18  
#define PARAM_ANEMO_OFFSET3 19  
#define PARAM_ANEMO_OFFSET4 20  
#define PARAM_ANEMO_FACTOR1 21  // anemometer calibration factor: conversion between gas flux (of air) and digital unit
#define PARAM_ANEMO_FACTOR2 22  
#define PARAM_ANEMO_FACTOR3 23  
#define PARAM_ANEMO_FACTOR4 24  

// Input/Output
#define ANEMOMETER_WRITE            I2C_FLUX
#define ANEMOMETER_READ             I2C_FLUX
#define  TAP_GAS1                   PWM2
//#define  TAP_GAS2                   IO2
//#define  TAP_GAS3                   PWM3
//#define  TAP_GAS4                   IO3

// Parameters stored in memory
#ifdef TAP_GAS1  
#define PARAM_FLUX_GAS1            4
#define PARAM_AVG_FLUX_GAS1        8
#define PARAM_DESIRED_FLUX_GAS1    45
#endif

#ifdef  TAP_GAS2
#define PARAM_FLUX_GAS2            5
#define PARAM_AVG_FLUX_GAS2        9
#define PARAM_DESIRED_FLUX_GAS2    46
#endif

#ifdef  TAP_GAS3
#define PARAM_FLUX_GAS3            6
#define PARAM_AVG_FLUX_GAS3        10
#define PARAM_DESIRED_FLUX_GAS3    47
#endif

#ifdef  TAP_GAS4
#define PARAM_FLUX_GAS4            7
#define PARAM_AVG_FLUX_GAS4        11     
#define PARAM_DESIRED_FLUX_GAS4    48
#endif

//few hard coded parameters for flux control
#define PARAM_FLUX_TOLERANCE             49    //define a tolerance of 1 cc/min
#define PARAM_FLUX_TIME_WINDOWS          50    //define a control windows of 10sec for the flux
#endif

/******************
 * FLAG DEFINITION
 ******************/

#define PARAM_STATUS       25

#define FLAG_STEPPER_CONTROL     0   // need to be set to 1 for control of engine
#define FLAG_PH_CONTROL          1   // set the condition to disable targeted modules when pumping is performed
#define FLAG_GAS_CONTROL         2
#define FLAG_FOOD_CONTROL        3   // need to be set to 1 for control of food

#define FLAG_PH_CALIBRATE        4

#define FLAG_RELAY_FILLING       8
#define FLAG_RELAY_EMPTYING      9
#define FLAG_RELAY_NOTUSED1      10
#define FLAG_RELAY_NOTUSED2      11
#define RELAY_PUMP_SHIFT         8 // We need to shift of 4 bits to get the value to send to relay board

#define FLAG_RELAY_ACID          12
#define FLAG_RELAY_BASE          13
#define FLAG_RELAY_NOTUSED3      14
#define FLAG_RELAY_NOTUSED4      15
#define RELAY_TAP_SHIFT          12 // We need to shift of 4 bits to get the value to send to relay board


/*********
 * Autoreboot parameters
 *********/
#define AUTOREBOOT 36000 // we will reboot automatically every 1h ... bad trick to prevent some crash problems of ethernet ...
uint16_t autoreboot=0;
// the delay may be prolongated if we received request on the ethernet



/*********
 * SETUP
 *********/

void setup() {
  delay(2000);
  Serial.begin(9600);
  delay(1000);
  setupParameters();
  
  pinMode(PWM4,OUTPUT);
  digitalWrite(PWM4,LOW);
  pinMode(IO4,OUTPUT);
  digitalWrite(IO4,LOW);
  
  
#ifdef THR_LINEAR_LOGS
  setupMemory();
  recoverLastEntryN();
  loadLastEntryToParameters();
#endif

  setSafeConditions(false);
  nilSysBegin();

}

void loop() {

}
