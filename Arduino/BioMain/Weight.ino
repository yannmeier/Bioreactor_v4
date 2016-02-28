#ifdef FOOD_CTRL

#define WEIGHT_STATUS_NORMAL    0
#define WEIGHT_STATUS_WAITING   1
#define WEIGHT_STATUS_EMPTYING  2
#define WEIGHT_STATUS_FILLING   3
#define WEIGHT_STATUS_STANDBY   4
#define WEIGHT_STATUS_ERROR     7

//#define EVENT_LOGGING 
//#define WEIGHT_DEBUG 1 //define this if you want to display the measured input from weight sensor
//#define EXTERNAL_WEIGHT_DEBUG

#ifdef I2C_RELAY_FOOD
int cycleCounter=0;
#define PUMP_SLOWDOWN_RATIO          10  // this would open for 5s and then wait the ratio
#endif

#ifdef WEIGHT_DEBUG
NIL_WORKING_AREA(waThreadWeight, 64);    // minimum of 32 !
#else
NIL_WORKING_AREA(waThreadWeight, 32);    // minimum of 32 !
#endif

NIL_THREAD(ThreadWeight, arg) {
  
  nilThdSleepMilliseconds(2000);
  
  //master pin is for communication with the Gas flux control board
  #ifdef MASTER_PIN
    pinMode(MASTER_PIN,OUTPUT);
    pinMode(MASTER_PWM,INPUT);
    digitalWrite(MASTER_PIN,LOW);
  #endif
  
  //read or get back the last value ?
  //int weight = (int)((float)getParameter(PARAM_WEIGHT_FACTOR)/(float)1000.0*(float)(analogRead(WEIGHT))-(float)getParameter(PARAM_WEIGHT_OFFSET));
  int weight = getParameter(PARAM_WEIGHT);
  byte weight_status=0;
  unsigned long tsinceLastEvent=0; // when was the last food cycle
  unsigned long lastCycleMillis=millis(); // when was the last food cycle

  //stop pumping on startup and put down the flags
  #ifdef FOOD_IN
    pinMode(FOOD_IN, OUTPUT);
    digitalWrite(FOOD_IN,LOW);
  #endif
  #ifdef FOOD_OUT
    pinMode(FOOD_OUT, OUTPUT);
    digitalWrite(FOOD_OUT,LOW);
  #endif

  all_off();   
        
  //get to the last log status, in the worst case just 18 seconds lag with current state
  //(1log every 10 seconds and watchdog with 8 seconds to reset)
  if (getParameter(PARAM_WEIGHT_STATUS)!=-1) {
    weight_status=(((uint16_t)getParameter(PARAM_WEIGHT_STATUS)) >> 13);
    tsinceLastEvent=(((uint16_t)getParameter(PARAM_WEIGHT_STATUS))&0b0001111111111111)*60000;
  }
  else
    weight_status=WEIGHT_STATUS_ERROR;

  while(true){ 
    
    //sensor read 
    #ifdef MODE_CALIBRATE
      nilThdSleepMilliseconds(10); //in calibration mode, we have much faster weight changes.
    #else
      nilThdSleepMilliseconds(500);
    #endif
    
    //moving average calibrated weight (parameters 'P' and 'Q' for factor and offset)
    weight = (int)((float)0.95*weight+0.05*((float)getParameter(PARAM_WEIGHT_FACTOR)/(float)1000.0*analogRead(WEIGHT)-(float)getParameter(PARAM_WEIGHT_OFFSET)));
    
    #ifdef EXTERNAL_WEIGHT_DEBUG
      wireWrite(110,0b10010000);
      nilThdSleepMilliseconds(6);
      Serial.println(wireReadFourBytesToInt(110));
    #endif
    
    
    /***********************************************
             Standby and Error management
    ************************************************/
    //flag down stands for deactivated weight control   
    if(getParameterBit(PARAM_STATUS, FLAG_FOOD_CONTROL)==0) 
      weight_status=WEIGHT_STATUS_STANDBY;
    
    //if weight control is on this first loop manages the standby and error cases (wake-up cases)
    else {
      if(weight_status==WEIGHT_STATUS_STANDBY){ 
        weight_status=WEIGHT_STATUS_NORMAL;
        Serial.println(F("sb>ok"));
      }

      if (weight<getParameter(PARAM_MIN_ABSOLUTE_WEIGHT) || weight>getParameter(PARAM_MAX_ABSOLUTE_WEIGHT)) {
        all_off();
        weight_status=WEIGHT_STATUS_ERROR;
        #ifdef EVENT_LOGGING
          writeLog(EVENT_WEIGHT_FAILURE,0);
        #endif
      } 

      else if (weight_status==WEIGHT_STATUS_ERROR) {
          #ifdef EVENT_LOGGING
            writeLog(EVENT_WEIGHT_BACK_TO_NORMAL,0);
          #endif
          weight_status=WEIGHT_STATUS_NORMAL;
          Serial.println(F("er>ok"));
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
      //setting the operation mode
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_PH_CONTROL);        //pH      ON
      setParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL);   //stepper ON
      #ifdef MASTER_PIN
        digitalWrite(MASTER_PIN,HIGH);                   //gas control ON
      #endif
      //switching to Sedimentation phase
      if(( (uint16_t)(tsinceLastEvent/60000))>=getParameter(PARAM_MIN_FILLED_TIME)){
        weight_status=WEIGHT_STATUS_WAITING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_WAITING, 0);  //need to move the event loggers
          writeLog(EVENT_MOTOR_STOP, 0);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_WAITING:
      //setting the operation mode
      all_off();
      //switching to Emptying phase
      if(( (uint16_t)(tsinceLastEvent/60000))>=getParameter(PARAM_SEDIMENTATION_TIME)){
        weight_status=WEIGHT_STATUS_EMPTYING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_EMPTYING_START, 0);  //need to move the event loggers
          nilThdSleepMilliseconds(10);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_EMPTYING:
      //setting the operation mode
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING);  //emptying ON
      //switching fo Filling phase
      if (weight<=getParameter(PARAM_WEIGHT_MIN)) {
        weight_status=WEIGHT_STATUS_FILLING;
        tsinceLastEvent=0; 
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_EMPTYING_STOP, 0);  //need to move the event loggers
          nilThdSleepMilliseconds(10);
        #endif
        // TURN ON ROTATION
        #ifdef EVENT_LOGGING
          writeLog(EVENT_MOTOR_START, 0);  //need to move the event loggers
          nilThdSleepMilliseconds(10);
        #endif
        // turn on filling pump
        #ifdef EVENT_LOGGING
          writeLog(EVENT_PUMPING_FILLING_START, 0);   //need to move the event loggers
          nilThdSleepMilliseconds(10);
        #endif
      }
      break;
      
    case WEIGHT_STATUS_FILLING:
      all_off();
      setParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING);     //filling  ON
      setParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL);   //stepper  ON
      #ifdef MASTER_PIN
        digitalWrite(MASTER_PIN,HIGH);                       //gas ctrl ON
      #endif
     
      if (weight>=getParameter(PARAM_WEIGHT_MAX)) {
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
    

    // There are 2 ways to control the food. Based on I2C relay or directly connected board with 2 relays   --> note that the I2C elays will be removed definitely on next version of the code
    // If it is I2C, the process I2C will take care of it ... otherwise we need to take care of it here   
    #ifdef FOOD_IN 
      digitalWrite(FOOD_IN, getParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING));
      delay(10);
      //nilThdSleepMilliseconds(50);
    #endif
    #ifdef FOOD_OUT 
      digitalWrite(FOOD_OUT, getParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING));
      delay(10);
      //nilThdSleepMilliseconds(50);
    #endif

    tsinceLastEvent+=(millis()-lastCycleMillis);
    lastCycleMillis=millis();
  }
}

#endif



/********************
    Utilities
*********************/

void all_off(){
  clearParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL); //stepper  OFF
  clearParameterBit(PARAM_STATUS, FLAG_PH_CONTROL);      //pH       OFF
  clearParameterBit(PARAM_STATUS, FLAG_RELAY_FILLING);   //filling  OFF
  clearParameterBit(PARAM_STATUS, FLAG_RELAY_EMPTYING);  //emptying ON
  #ifdef MASTER_PIN
    digitalWrite(MASTER_PIN,LOW);
  #endif
}














