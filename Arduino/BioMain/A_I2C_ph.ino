/*************
 * PH METER
 ************/
#ifdef PH_CTRL

  /**************
    DEBUG FLAGS
  ***************/
  //#define DEBUG_PH            1 // DISPLAY MEASURE AND STATE
  //#define CALIBRATION_PRINT   1 // DISPLAY TEXT FOR CALIBRATION
  
  /********************************
    Internal state of ph controler 
  ********************************/
  #define TAP_STATE_CLOSED      11 
  #define TAP_STATE_ADDING_ACID 12
  #define TAP_STATE_ADDING_BASE 13
  
  /********************************
    External state of ph controler 
  ********************************/
  // changed through "setParameter(PARAM_PH_STATE)"
  #define PH_STATE_PAUSE    0
  #define PH_STATE_CTRL     1
  #define PH_STATE_PURGE    2
  #define PH_STATE_OFF      3
  #define PH_STATE_CALIBRATION     4
 
  /**********************
    PID of ph controler 
  **********************/
  #define Kp       0.5     
  #define Ki       0.1
  #define Kd       0.5
  
  /****************************
      PH TABLE INDEXES
  *****************************/
  #ifdef PH_TABLE
    #define VALUE   1
    #define MEASURE 2
    #define PH4     3
    #define PH7     4
    #define STATE   5
  #endif
  
    /****************************
      TAP TABLE INDEXES
  *****************************/
  #ifdef PH_TABLE
    #define TIMEOPEN   1
    #define TIMECLOSED 2
    #define TAP_STATE  3
  #endif


/*************************************
   initialized pH and tap structure
**************************************/
#ifdef PH_STRUCT  
   struct MypH{
    float value;
    int measure;
    int ph4=671;
    int ph7=1038;
    //int ph10=1411;
    byte state=0;
  };

  struct Mytap{
    unsigned long timeSinceOpen=millis(); 
    unsigned long timeSinceLocked=millis(); //should't it be in seconds ?
    byte state= TAP_STATE_CLOSED;
  };
#endif

/******************************
           Main Thread
*******************************/
#ifdef PH_CTRL_OLD
#ifdef DEBUG_PH
    NIL_WORKING_AREA(waThreadPH, 190);
#else
    NIL_WORKING_AREA(waThreadPH, 0);  //check what memory is necessary 
#endif   
NIL_THREAD(ThreadPH, arg){
  MypH ph;
  Mytap tap;
  pH_init();
  nilThdSleepMilliseconds(10000); //wait for sensor to "preheat"
  while(true){  
      nilThdSleepMilliseconds(500);
      pH_state_machine(&ph,&tap);  
  }
}
#endif

/********************************************************
           State Machine with TABLE type
*********************************************************/
#ifdef PH_TABLE
void pH_state_machine(int *ph, unsigned long *tap){
  
    ph[MEASURE] = getpH();
    nilThdSleepMilliseconds(100);
    ph[STATE] = getParameter(PARAM_PH_STATE); 
    //not controlling while pumping
    #ifdef FOOD_CTRL
    if(getParameterBit(PARAM_STATUS, FLAG_PH_CONTROL) == 1){
    #endif    
    
    // check if inside reasonnable bound
    if((ph[MEASURE] > 0) || (ph[MEASURE] < 2100))
    {
      #ifdef DEBUG_PH
        Serial.println(F("ph state:"));
        Serial.println(ph[STATE]);
        Serial.println(F("ph value:"));
        Serial.println(getpH_value());
        Serial.println(F("ph measure:"));
        Serial.println(ph[MEASURE]);
      #endif
      
      
      switch (ph[STATE]){
        case PH_STATE_CTRL:
          ph[VALUE] = getpH_value(); 
          nilThdSleepMilliseconds(100);
          #if defined (TAP_ACID) && (TAP_BASE)      
            control_ph(tap,ph);
          #endif
          break;

        case PH_STATE_OFF:
          #if defined (TAP_ACID) && (TAP_BASE)  
          close_taps();
          #endif
        break;

        case PH_STATE_PAUSE:
          #if defined (TAP_ACID) && (TAP_BASE)  
          close_taps();
          #endif
          ph[VALUE] = getpH_value();
          break;
          
        case PH_STATE_PURGE:
          #if defined (TAP_ACID) && (TAP_BASE)  
          purge();
          while(getParameter(PARAM_PH_STATE) == PH_STATE_PURGE)
          {
            nilThdSleepMilliseconds(250);
          }
          close_taps();
          #endif
          break;
        
        case PH_STATE_CALIBRATION:
          #if defined (TAP_ACID) && (TAP_BASE)  
            close_taps();
          #endif
          #ifdef FOOD_CTRL
            setParameterBit(PARAM_STATUS, FLAG_PH_CALIBRATE);
          #endif
          //interactive calibration not finished
          //get_calibration_parameters(ph);
          //calibrate(ph);
          #ifdef CALIBRATION_PRINT
            Serial.println(F("End of calibration"));
            Serial.println(getParameter(PARAM_PH_FACTOR_A));
            Serial.println(getParameter(PARAM_PH_FACTOR_B));      
          #endif
          #ifdef FOOD_CTRL
            clearParameterBit(PARAM_STATUS, FLAG_PH_CALIBRATE);
          #endif
          break;
          
          default :
          #ifdef DEBUG_PH
            Serial.println(F("Wrong state"));
          #endif
          break;
      }
    }
    //sensor value out of limits
    else{
      #ifdef DEBUG_PH
        Serial.println(F("Sensor Error"));
      #endif
      close_taps();
    }
    #ifdef DEBUG_PH
      Serial.flush();
    #endif
    
  #ifdef FOOD_CTRL
  }  
  else
     #if defined (TAP_ACID) && (TAP_BASE)  
       close_taps();
     #endif
  #endif
}
#endif 
/********************************************************
           State Machine with STRUCT type
*********************************************************/

#ifdef PH_STRUCT
void pH_state_machine(struct MypH* ph, struct Mytap* tap){
  
    ph->measure = getpH();
    nilThdSleepMilliseconds(100);
    ph->state = getParameter(PARAM_PH_STATE); 
    //not controlling while pumping
    #ifdef FOOD_CTRL
    if(getParameterBit(PARAM_STATUS, FLAG_PH_CONTROL) == 1){
    #endif    
    
    // check if inside reasonnable bound
    if((ph->measure > 0) || (ph->measure < 2100))
    {
      #ifdef DEBUG_PH
        Serial.println(F("ph state:"));
        Serial.println(ph->state);
        Serial.println(F("ph value:"));
        Serial.println(getpH_value());
        Serial.println(F("ph measure:"));
        Serial.println(ph->measure);
      #endif
      
      
      switch (ph->state){
        case PH_STATE_CTRL:
          ph->value = getpH_value(); 
          nilThdSleepMilliseconds(100);
          #if defined (TAP_ACID) && (TAP_BASE)      
            control_ph(tap,ph);
          #endif
          break;

        case PH_STATE_OFF:
          #if defined (TAP_ACID) && (TAP_BASE)  
          close_taps();
          #endif
        break;

        case PH_STATE_PAUSE:
          #if defined (TAP_ACID) && (TAP_BASE)  
          close_taps();
          #endif
          ph->value = getpH_value();
          break;
          
        case PH_STATE_PURGE:
          #if defined (TAP_ACID) && (TAP_BASE)  
          purge();
          while(getParameter(PARAM_PH_STATE) == PH_STATE_PURGE)
          {
            nilThdSleepMilliseconds(250);
          }
          close_taps();
          #endif
          break;
        
        case PH_STATE_CALIBRATION:
          #if defined (TAP_ACID) && (TAP_BASE)  
            close_taps();
          #endif
          #ifdef FOOD_CTRL
            setParameterBit(PARAM_STATUS, FLAG_PH_CALIBRATE);
          #endif
          //interactive calibration not finished
          get_calibration_parameters(ph);
          calibrate(ph);
          #ifdef CALIBRATION_PRINT
            Serial.println(F("End of calibration"));
            Serial.println(getParameter(PARAM_PH_FACTOR_A));
            Serial.println(getParameter(PARAM_PH_FACTOR_B));      
          #endif
          #ifdef FOOD_CTRL
            clearParameterBit(PARAM_STATUS, FLAG_PH_CALIBRATE);
          #endif
          break;
          
          default :
          #ifdef DEBUG_PH
            Serial.println(F("Wrong state"));
          #endif
          break;
      }
    }
    //sensor value out of limits
    else{
      #ifdef DEBUG_PH
        Serial.println(F("Sensor Error"));
      #endif
      close_taps();
    }
    #ifdef DEBUG_PH
      Serial.flush();
    #endif
    
  #ifdef FOOD_CTRL
  }  
  else
     #if defined (TAP_ACID) && (TAP_BASE)  
       close_taps();
     #endif
  #endif
}
#endif

/***************************************************
                   Sensor read
****************************************************/

//get pH value on I2C
int getpH() {
    if (wireDeviceExists(I2C_PH)) {
      int sum=0;
      byte i=0;
      //built-in average
      for(i=0; i<6;i++){
        wireWrite(I2C_PH,0b10010000);
        delay(6); //try without sleep
        //nilThdSleepMilliseconds(6);
        sum += wireReadFourBytesToInt(I2C_PH);
        //delay(10); 
      }
      sum = sum/i;
      return sum;
    }
    else{
      //could be anything above PH_INTERCEPT, 
      //just to produce a negative pH value->error
      return 3000;  
    }
  }
 
// convert binary to readable pH
int getpH_value(){
    // value = slope*(measure + intersept)
    // default slope is float 6.685 --> saved as int 6685  
    int value = (int) ((((float)getParameter(PARAM_PH_FACTOR_A))/((float)1000))*(float)(getpH() + getParameter(PARAM_PH_FACTOR_B)));
    setParameter(PARAM_PH, value);
    return value;    
}  

#if defined (TAP_ACID) && (TAP_BASE)    
/*************************************
            Utilities
**************************************/

void pH_init(){
  #if defined (TAP_ACID) && (TAP_BASE)   
  pinMode(TAP_ACID, OUTPUT);
  pinMode(TAP_BASE, OUTPUT);
  digitalWrite(TAP_ACID, LOW);
  digitalWrite(TAP_BASE, LOW);
  #endif
  #ifdef FOOD_CTRL
  clearParameterBit(PARAM_STATUS, FLAG_PH_CALIBRATE);
  #endif
}


#if defined (TAP_ACID) && (TAP_BASE)  
void purge(){
  open_tap(TAP_BASE);
  open_tap(TAP_ACID);
}
  
  void close_taps(void){
  close_tap(TAP_BASE);
  close_tap(TAP_ACID);
}
  
void open_tap(int _tap){
  digitalWrite(_tap,HIGH); 
}

void close_tap(int _tap){
    digitalWrite(_tap, LOW);
}
#endif

unsigned long time_elapsed (unsigned long tpast){
  return (millis() - tpast);
}

/*************************
    Dependecies
**************************/
  
  //consider a chemical buffering time PH_ADJUST_DELAY
  boolean tap_Ready(unsigned long timeSinceLocked){
    if(time_elapsed(timeSinceLocked ) > (unsigned long)(getParameter(PARAM_PH_ADJUST_DELAY)))
      return 1;
    else
      return 0;
  }

  // calculate required tap opening time
  unsigned int pid_controler_ph(){
    int sum_of_error = 0;
    static int previous_error = 0;
    int error = ( getParameter(PARAM_TARGET_PH) - getParameter(PARAM_PH));
    if(error < 0)
      error += getParameter(PARAM_PH_TOLERANCE);
    else if(error > 0)
        error -= getParameter(PARAM_PH_TOLERANCE); 

    /// Integral term + anti windup ///
    sum_of_error = error + previous_error;
    if(sum_of_error < -5000)
      sum_of_error = -5000;
    else if(sum_of_error > 5000)
        sum_of_error = 5000;
        
    /// Derivative term   ///
    unsigned int t = abs(error * Kp  + sum_of_error * Ki + 
      (error - previous_error) * Kd);

    #ifdef DEBUG_PH
      Serial.print(F("e= "));
      Serial.println(error);
      Serial.print(F("se="));
      Serial.println(sum_of_error);
      Serial.print(F("de="));
      Serial.println((error - previous_error));
      Serial.print(F("t= "));
      Serial.println(t);
    #endif

    if (t < 50){
      t = 50;
    }
    return t;
  }

  #endif
/*************************************
      Calibration function
*************************************/

  //set PARAM_PH_FACTOR_A and PARAM_PH_FACTOR_B
  void calibrate(MypH * ph){
    // 3 points linear regression
    /*
    float y_mean =(getParameter(PARAM_REF_PH4) + getParameter(PARAM_REF_PH7) + getParameter(PARAM_REF_PH10)) /2 ; 
    float x_mean = (ph->ph4 + ph->ph7 ) / 2;

    float nominateur = (ph->ph4 - x_mean)*(getParameter(PARAM_REF_PH4) - y_mean) +
     (ph->ph10 - x_mean)*(getParameter(PARAM_REF_PH10) - y_mean) +
     (ph->ph7 - x_mean)*(getParameter(PARAM_REF_PH7) - y_mean);
    // denominator is stored in variable slope
    float slope = sq(ph->ph4 - x_mean) + sq(ph->ph7 - x_mean) + sq(ph->ph10 - x_mean);
    slope = nominateur/slope;
    
    float intesept =  - (x_mean -  y_mean/slope);
    
    #ifdef DEBUG_PH 
      Serial.println(y_mean);
      Serial.println(x_mean);
      Serial.println(intesept);
      Serial.println(slope);
    #endif
    setAndSaveParameter(PARAM_PH_FACTOR_A, slope*1000);// 6.668 FLOAT TO 6668 INT 
    setAndSaveParameter(PARAM_PH_FACTOR_B, intesept);*/
    
    float slope = (float)(7000-4000)/((float)getParameter(PARAM_REF_PH7)-(float)getParameter(PARAM_REF_PH4));
    float offset = -1*(((float)getParameter(PARAM_REF_PH7)-(float)7000/slope)+((float)getParameter(PARAM_REF_PH4)-(float)4000/slope))/2;
    
    setAndSaveParameter(PARAM_PH_FACTOR_A, (int)(slope*1000));// 6.668 FLOAT TO 6668 INT 
    setAndSaveParameter(PARAM_PH_FACTOR_B, (int)offset);
  }

#endif
