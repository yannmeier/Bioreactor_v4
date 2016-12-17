#if defined(WEIGHT_DATA) && defined(WEIGHT_CLK)

#include "HX711.h"

#define calibration_factor -7050.10 //From SparkFun_HX711_Calibration sketch
HX711 scale(WEIGHT_DATA, WEIGHT_CLK);


#define WEIGHT_STATUS_NORMAL    0
#define WEIGHT_STATUS_WAITING   1
#define WEIGHT_STATUS_EMPTYING  2
#define WEIGHT_STATUS_FILLING   3
#define WEIGHT_STATUS_STANDBY   4
#define WEIGHT_STATUS_ERROR     7

#define EVENT_LOGGING 
//#define WEIGHT_DEBUG 1 

#ifdef WEIGHT_DEBUG
NIL_WORKING_AREA(waThreadWeight, 96);    
#else
NIL_WORKING_AREA(waThreadWeight, 56); // minimum of 32 !
#endif

NIL_THREAD(ThreadWeight, arg) {
  /********************************************
               initialisation
  ********************************************/             
  int weight = int(getParameter(PARAM_WEIGHT));
  byte weight_status=0;
  byte previous_status= WEIGHT_STATUS_ERROR;
  unsigned long tsinceLastEvent=0;        
  unsigned long lastCycleMillis=millis(); // when was the last food cycle
  all_off();                              //clear flags, shut down pumps
  scale.set_scale(calibration_factor);    //Default from SparkFun_HX711_Calibration sketch  
  nilThdSleepMilliseconds(2000);

  //get to the last log status, worst case: just 18 seconds lag with current state
  if (getParameter(PARAM_WEIGHT_STATUS)!=-1) {
    weight_status=(((uint16_t)getParameter(PARAM_WEIGHT_STATUS)) >> 13);
    tsinceLastEvent=(((uint16_t)getParameter(PARAM_WEIGHT_STATUS))&0b0001111111111111)*60000;
  }
  else
    weight_status=WEIGHT_STATUS_ERROR;
    
  /********************************************
               Thread Loop
  ********************************************/
  while(true){ 
    //sensor read 
    #ifdef MODE_CALIBRATE
      nilThdSleepMilliseconds(10); //in calibration mode, we have much faster weight changes. ---> remove that and change for dynamic calibration
    #else
      nilThdSleepMilliseconds(1000);
    #endif
    if(weight_status !=WEIGHT_STATUS_ERROR) previous_status=weight_status;
    while(!scale.is_ready()) {
    	nilThdSleepMilliseconds(10);
    }
    protectThread();
    weight = 3*scale.get_units(); //to be improved (change calib values for the HX711)
    unprotectThread();

    /***********************************************
             Standby and Error management
    ************************************************/
    //flag down stands for deactivated weight control   
    //if weight control is on this first loop manages the standby and error cases (wake-up cases)
    if(getParameterBit(PARAM_STATUS, FLAG_FOOD_CONTROL)==0){ 
      weight_status=WEIGHT_STATUS_STANDBY;
    }else { 
      
      if(weight_status==WEIGHT_STATUS_STANDBY){ 
        weight_status=WEIGHT_STATUS_NORMAL;
      #ifdef WEIGHT_DEBUG
        Serial.println(F("sb>ok"));
      #endif
      }
      //safety measures
      if (weight<(0.80*getParameter(PARAM_WEIGHT_MIN)) || weight>(1.20*getParameter(PARAM_WEIGHT_MAX))) {
        all_off();
        weight_status=WEIGHT_STATUS_ERROR;
        
        #ifdef WEIGHT_DEBUG
          Serial.print(F("wght err:"));
          Serial.println(weight);
        #endif
        
        #ifdef EVENT_LOGGING
          writeLog(EVENT_WEIGHT_FAILURE,weight);
        #endif
      } else if (weight_status==WEIGHT_STATUS_ERROR) {
          #ifdef EVENT_LOGGING
            writeLog(EVENT_WEIGHT_BACK_TO_NORMAL,0);
          #endif
          if(previous_status==WEIGHT_STATUS_ERROR) weight_status=WEIGHT_STATUS_NORMAL;
          else weight_status=previous_status;
          #ifdef WEIGHT_DEBUG
            Serial.println(F("er>ok"));
          #endif
      }
    }

    setParameter(PARAM_WEIGHT, weight);
    setParameter(PARAM_WEIGHT_STATUS, (((uint16_t)(tsinceLastEvent/60000)) | ((uint16_t)(weight_status<<13))));
    
   #ifdef WEIGHT_DEBUG
        Serial.print(weight_status);
        Serial.print(F(" "));
        Serial.println(weight);
    #endif
  
    /**************************************
           Weight State Machine
    **************************************/
    switch (weight_status) {
      
    case WEIGHT_STATUS_NORMAL: 
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_PH_CONTROL);        //pH      ON
      setParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL);   //stepper ON    
      if(( (uint16_t)(tsinceLastEvent/60000))>=getParameter(PARAM_FILLED_TIME)){                //switch to Sedimentation 
        weight_status=WEIGHT_STATUS_WAITING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_WAITING, 0);  
          writeLog(EVENT_MOTOR_STOP, 0);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_WAITING:
      all_off();
      if(( (uint16_t)(tsinceLastEvent/60000))>=getParameter(PARAM_SEDIMENTATION_TIME)){        //switch to Emptying
        weight_status=WEIGHT_STATUS_EMPTYING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_EMPTYING_START, 0);  
          nilThdSleepMilliseconds(10);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_EMPTYING:
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING);  //emptying ON
      if (weight<=getParameter(PARAM_WEIGHT_MIN)) {        //switch fo Filling 
        #ifdef WEIGHT_DEBUG
          Serial.print(F("empty:"));
          Serial.println(weight);
        #endif
        weight_status=WEIGHT_STATUS_FILLING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_EMPTYING_STOP, 0);  
          nilThdSleepMilliseconds(10);
        #endif
        // TURN ON ROTATION
        #ifdef EVENT_LOGGING
          writeLog(EVENT_MOTOR_START, 0);    
          nilThdSleepMilliseconds(10);
        #endif
        // turn on filling pump
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_FILLING_START, 0);   
          nilThdSleepMilliseconds(10);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_FILLING:
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING);     //filling  ON
      setParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL);   //stepper  ON
     
      if (weight>=getParameter(PARAM_WEIGHT_MAX)) {
        Serial.print(F("full:"));
        Serial.println(weight);
        weight_status=WEIGHT_STATUS_NORMAL;
        tsinceLastEvent=0;
        
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_FILLING_STOP, 0);  //need to move the event loggers
          nilThdSleepMilliseconds(10);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_ERROR:
      all_off();
    }
    

    // Food control with 2 pumps   
    #ifdef FOOD_IN 
      digitalWrite(FOOD_IN, getParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING));
      delay(10);
    #endif
    #ifdef FOOD_OUT 
      digitalWrite(FOOD_OUT, getParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING));
      delay(10);
    #endif
    tsinceLastEvent+=(millis()-lastCycleMillis);
    lastCycleMillis=millis();
  } 
}


void processWeightCommand(char command, char* data, Print* output) {
  switch (command) {
  case 'e':

    break;
  case 'l':

    break;
  case 'h':

    break;
  case 'q':

   break;
  default:
	printWeightHelp(output);
  }

}
void printWeightHelp(Print* output) {
  output->println(F("Weight help"));
  output->println(F("(we) Empty level"));
  output->println(F("(wl) Low level"));
  output->println(F("(wh) High level"));
  output->println(F("(wq) Quantity (g) high"));
}



#endif

/********************
    Utilities
*********************/

void all_off(){
  //stop pumping on startup and put down the flags
  #ifdef FOOD_IN
    pinMode(FOOD_IN, OUTPUT);
    digitalWrite(FOOD_IN,LOW);
  #endif
  #ifdef FOOD_OUT
    pinMode(FOOD_OUT, OUTPUT);
    digitalWrite(FOOD_OUT,LOW);
  #endif
  clearParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL); //stepper  OFF
  clearParameterBit(PARAM_STATUS, FLAG_PH_CONTROL);      //pH       OFF
  clearParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING);   //filling  OFF
  clearParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING);  //emptying ON
}


