/************************************************
 *               PINS DEF
 ***********************************************/
 
//#define LCD_SELECT RXLED //pin SS (D8)
const int LCDD7 = A6;
const int LCDD6 = 12;
const int LCDD5 = 6;
const int LCDD4 = 8;
const int LCDE  = 9;
const int LCDRS = 10;

/************************************************
 *             GENERAL CONFIG
 ***********************************************/
 
#define MAX_PARAM  52
#define MAX_CONFIG_PARAM 4             // Maximum number of configurable parameters in the config menu
#define MAX_WEIGHT_PARAM 4             // Maximum number of parameters in the weight calibration menu

/************************************************
 *               PARAMS TABLE     
 ***********************************************/

// READ ONLY PARAMS
#define PARAM_TEMP_LIQ             0   // current temperature of the solution
#define PARAM_WEIGHT_G             5   // current weight in gr
#define PARAM_STATUS               25  // current status of the bioreactor

// CONFIGURABLE PARAMS
#define PARAM_STEPPER_SPEED        26  // motor speed in RPM
#define PARAM_TEMP_TARGET          3   // target temperature of the liquid
#define PARAM_SEDIMENTATION_TIME   31  // MINUTES to wait without rotation before emptying
#define PARAM_FILLED_TIME          32  // MINUTES to stay in the filled state

// WEIGHT CALIB
#define PARAM_WEIGHT_FACTOR        33  // Weight calibration: conversion factor digital -> gr (weight=FACTOR*dig_unit)
#define PARAM_WEIGHT_OFFSET        34  // Weight calibration: digital offset value when bioreactor is empty
#define PARAM_WEIGHT_MIN           7   // Minimum weight (in internal units)
#define PARAM_WEIGHT_MAX           8   // Maximum weight (in internal units)
#define PARAM_WEIGHT               4  // Weight in unit of balance (Not configurable. Utilitary only)

/************************************************
 *               ENCODER DEFINES
 ***********************************************/
 
#define ENCODER_CLOCKWISE      0
#define ENCODER_ANTI_CLOCKWISE 1
#define ENCODER_BUTTON         7
#define INT_CLOCKWISE          2
#define INT_ANTI_CLOCKWISE     3
#define INT_BUTTON             4  

/************************************************
 *                  MENU TYPES
 ***********************************************/
 
#define MENU_SELECTOR       0
#define MENU_SENSOR         1
#define MENU_CONFIG         2
#define MENU_CALIBRATION    3
#define MENU_SET_VALUE      4
#define MENU_WEIGHT_CONFIRM 5

/************************************************
 *        OUTPUT BUFFER SIZE IN BYTES
 ***********************************************/
 
#define OUT_BUF_SIZE    5
