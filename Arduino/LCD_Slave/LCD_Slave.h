/************************************************
 *               PINS DEF
 ***********************************************/
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
#define MAX_CONFIG_PARAM 7             // Maximum number of configurable parameters in the config menu
/************************************************
 *               PARAMS TABLE
 ***********************************************/
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
/************************************************
 *        OUTPUT BUFFER SIZE IN BYTES
 ***********************************************/
#define OUT_BUF_SIZE    5
