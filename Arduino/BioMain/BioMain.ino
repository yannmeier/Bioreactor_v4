/**************
 * LIBRAIRIES
 **************/
//MultiThread
#include <NilRTOS.h>
//Lib to access memory with SPI
#include <SPI.h>
// Library that allows to start the watch dog
#include <avr/wdt.h>
// http://www.arduino.cc/playground/Code/Time
// We need to deal with EPOCH
#include <Time.h>

/******************
 * DEFINE CARD TYPE
 ******************/
//ONLY PARAMETER THAT CHANGES 

#define TYPE_MAIN     1   // card to control the basic functions: food, motor, temperature
//#define TYPE_PH
//#define TYPE_GAS

/******************
 * OPERATION MODE
 ******************/
//if you choose the calibration mode, you have to select a GENERAL card type first! 
//(not the GAS card, even when you calibrate the anemometer)
//#define MODE_CALIBRATE    1 //In this mode, you start the interactive calibration process.

/********************
 * PIN&ADRESS MAPPING
 *********************/
//I2C addresses 
#define I2C_FLUX          106//B01101000 --> to be redefined (AT32u4 slave)
#define I2C_PH            104//B01101000 --> to be redefined (AT32u4 slave)

//Pin definition
#define D4   4  //temp probe
#define D6   6  //temp smd (? A4)
#define D10  10 //memory select
#define D11  11 //slave at32u4 for LCD
#define D12  12 //temp control
#define D13  13 //blink
#define D18  18 //stepper
#define D19  19 //stepper
#define D20  20 //food in
#define D21  21 //food out
#define D22  22 //weight data
#define D23  23 //weight clock


/**************************************
 * ACTIVE THREAD DEPENDING CARD TYPE
 **************************************/
#ifdef TYPE_MAIN
//#define THR_LORA         1
#define STEPPER            {D18,D19}
#define FOOD_CTRL          1
#define FOOD_IN            D20
#define FOOD_OUT           D21
#define WEIGHT_DATA        D22
#define WEIGHT_CLK         D23     //need to redefine the calibration parameters and process (see "HX711")
#define TEMPERATURE_CTRL   1
  #define TEMP_LIQ         D4
  #define TEMP_PCB         D6 
  #define TEMP_PID         D12
 #define THR_LINEAR_LOGS    1
  #define FLASH_SELECT     D10    //Flash SS_SPI 
#define THR_MONITORING     1  
  #define MONITORING_LED   D13
//  #define  LCD_SELECT       D11    //LCD screen SS_SPI
#endif


#ifdef TYPE_GAS
#define GAS_CTRL           1
#define THR_LINEAR_LOGS 	1
#define THR_MONITORING     1  // starts the blinking led and the watch dog counter
#define MONITORING_LED     13
#endif

#ifdef MODE_CALIBRATE
#define GAS_CTRL           1 
//undefine unused threads during calibration --> calibration mode to be removed for a dynamic one
#undef PH_CTRL 
#undef TAP_ACID
#undef TAP_BASE
#undef THR_STEPPER
#undef TEMPERATURE_CTRL
#endif

/***********************
 * SERIAL, LOGGER AND DEBUGGER
 ************************/

#define THR_SERIAL    1


/*******************************
 * THREADS AND PARAMETERS PRESENT IN EACH CARD 
 *******************************/

#ifdef THR_LINEAR_LOGS
#define LOG_INTERVAL          10  // define the interval in seconds between storing the log
//#define DEBUG_LOGS            1
#endif

/*******************************
 * CARD DEFINITION (HARD CODED)
 *******************************/
#ifdef STEPPER
  #define PARAM_STEPPER_SPEED       18   // motor speed, parameter S (!!!!!TO BE REPROGRAMMED IN RPM!!!!!!!)
#endif

#ifdef     TEMPERATURE_CTRL
  #define PARAM_TEMP_LIQ      0   // temperature of the solution
  #define PARAM_TEMP_PCB      1   // temperature of the heating plate
  #define PARAM_TEMP_TARGET   26  // target temperature of the liquid
#define PARAM_TEMP_MAX             27  // maximal temperature of the plate
#if defined (TEMP_PID) 
#define PARAM_TEMP_REG_TIME    28 //in [ms]
#endif
#endif

/*************************************/

#if defined(WEIGHT_DATA) && defined(WEIGHT_CLK) 
#define PARAM_WEIGHT_FACTOR        15  // Weight calibration: conversion factor digital -> gr (weight=FACTOR*dig_unit)
#define PARAM_WEIGHT_OFFSET        16  // Weight calibration: digital offset value when bioreactor is empty
#define PARAM_WEIGHT               2   // in gr
#define PARAM_WEIGHT_MIN           29    
#define PARAM_WEIGHT_MAX           30  
#define PARAM_SEDIMENTATION_TIME   35  // MINUTES to wait without rotation before emptying
#define PARAM_FILLED_TIME          36  // MINUTES to stay in the filled state
#define PARAM_WEIGHT_STATUS        23  // current STATUS // BBBAAAAA AAAAAAAA : A = wait time in minutes, B = status
#endif

/*************************************/

#ifdef    PH_CTRL
#define PARAM_PH            3    // current pH
#define PARAM_TARGET_PH     31   // desired pH
#define PARAM_PH_FACTOR_A   32
#define PARAM_PH_FACTOR_B   33
#define PARAM_PH_STATE      24  // 0: Pause 1 : normal acquisition, 2 : purge of pipes,  4: calibration pH=4, 7: calibration pH=7, 10: calibration pH=10
//#define PARAM_REF_PH4		12
//#define PARAM_REF_PH7		13  --> TO BE REPLACED BY TEMPORRY VALUES IN THE CODE	
//#define PARAM_REF_PH10	14	

//not parameters, hard coded values, set the minimal delay between pH adjustements to 10 seconds --> ???
#define PARAM_PH_ADJUST_DELAY   38    //delay between acid or base supplies
#define PARAM_PH_OPENING_TIME   39    //1sec TAP opening when adjusting
#define PARAM_PH_TOLERANCE      34    //correspond to a pH variation of 0.1
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

#ifdef  TAP_GAS2busy
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

#define FLAG_RELAY_ACID          12
#define FLAG_RELAY_BASE          13

#define SERIAL_BUFFER_LENGTH    36
#define SERIAL_MAX_PARAM_VALUE_LENGTH  32

/*********
 * SETUP
 *********/
void setup() {
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  setupParameters(); 
  #ifdef FLASH_SELECT 
    pinMode(FLASH_SELECT,OUTPUT);
  setupMemory();
  recoverLastEntryN();
  loadLastEntryToParameters();   //get back the previous config  
  #endif

  #ifdef LCD_SELECT              //disable SPI modules 
    setupLCD();
  #endif
  
  setSafeConditions(false);
  nilSysBegin();
}

void loop() {}
