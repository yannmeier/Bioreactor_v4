#ifdef TEMP_PID_COLD
#ifdef TEMPERATURE_CTRL

#include <PID_v1.h>

void pid_ctrl();
void heatingSetup();

int count=0;
double heatingRegInput;
double heatingRegOutput;
double heatingRegSetpoint;
double coolingRegInput;
double coolingRegOutput;
double coolingRegSetpoint;

unsigned long heatingRegWindowStartTime;
//Specify the heating regulation links and initial tuning parameters //Kp=100; Ti=0.2 Td=5 are initial testing param.
//PID object definition can be found in PID library (to include for compilation)
  PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 13,0.18,600, DIRECT);
  PID coolingRegPID(&coolingRegInput, &coolingRegOutput, &coolingRegSetpoint, 7000,15,1600, DIRECT);


NIL_WORKING_AREA(waThread_HOT_N_COLD, 32); // minimum of 16 The momory change with time
NIL_THREAD(Thread_HOT_N_COLD, arg) 
{
  

  nilThdSleepMilliseconds(5000); 
  pinMode(TEMP_PID_HOT, OUTPUT);
  #ifdef TEMP_PID_COLD
  pinMode(TEMP_PID_COLD, OUTPUT);
  #endif
  //Todo : update heatingSetup when a parameter is changed
  heatingSetup();
  
  while(TRUE){
    
    mode_auto_switch();
    
     //Debut
     if(count%30==0)
      Serial.print(getParameter(PARAM_TEMP_SAMPLE));
    count=(count+1)%101;
    pid_ctrl();
    nilThdSleepMilliseconds(300);  //refresh every 500ms
  }
}


/*Temperature PID Control addressing relay*/

void pid_ctrl()
{
  float exactPresentTime;
  
  if(getParameter(PARAM_PID_STATUS)==0)
  {
   // PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 7000,15,300, DIRECT);
  digitalWrite(TEMP_PID_COLD, LOW); 
  heatingRegInput = getParameter(PARAM_TEMP_SAMPLE);
  heatingRegSetpoint = getParameter(PARAM_TARGET_LIQUID_TEMP);
  heatingRegPID.Compute();        // the computation takes only 30ms!
  // turn the output pin on/off based on pid output
  exactPresentTime = millis();
  }
  else if((getParameter(PARAM_PID_STATUS)==1)||(getParameter(PARAM_PID_STATUS)==2))
  {
    digitalWrite(TEMP_PID_HOT, LOW); 
    coolingRegInput = -1*getParameter(PARAM_TEMP_SAMPLE);
    coolingRegSetpoint = -1*getParameter(PARAM_TARGET_LIQUID_TEMP);
    coolingRegPID.Compute();        // the computation takes only 30ms!
    // turn the output pin on/off based on pid output
    exactPresentTime = millis();
  }
  
  
  if (exactPresentTime - heatingRegWindowStartTime > getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW)) { 
    //time to shift the Relay Window
    heatingRegWindowStartTime += getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW);
  }

  if(getParameter(PARAM_PID_STATUS)==0)
  {
  
  if((heatingRegOutput > exactPresentTime - heatingRegWindowStartTime) 
    && (getParameter(PARAM_TEMP_PLATE)<getParameter(PARAM_TEMP_MAX))
    && (getParameter(PARAM_TEMP_PLATE)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_SAMPLE)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_PLATE) != 0xFF)
    && (getParameter(PARAM_TEMP_SAMPLE)   !=0xFF))
  {
    digitalWrite(TEMP_PID_HOT, HIGH); 
  }  
  else 
  {
    digitalWrite(TEMP_PID_HOT, LOW); 
  } 

  }
  
 else if((getParameter(PARAM_PID_STATUS)==1)||(getParameter(PARAM_PID_STATUS)==2)){
   if((coolingRegOutput > exactPresentTime - heatingRegWindowStartTime) /*
    && (getParameter(PARAM_TEMP_LIQ)<getParameter(PARAM_TEMP_MAX))
    && (getParameter(PARAM_TEMP_LIQ)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_SAMPLE)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_SAMPLE)>getParameter(PARAM_MIN_TEMPERATURE))
    && (getParameter(PARAM_TEMP_LIQ) != 0xFF)
    && (getParameter(PARAM_TEMP_SAMPLE)   !=0xFF)*/)
  {
    digitalWrite(TEMP_PID_COLD, HIGH); 
  }  
  else 
  {
    digitalWrite(TEMP_PID_COLD, LOW); 
  } 
  
}
}

// see the rest of oliver's code for sanity checks

void heatingSetup()
{
  //tell the PID to range between 0 and the full window size
  heatingRegPID.SetOutputLimits(0, getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW));          //what is heating regulation time windows ???
  //turn the PID on, cf. PID library
  heatingRegPID.SetMode(AUTOMATIC);                 
  //set PID sampling time to 10000ms                   //possibly set a timer condition with a nilsleep instead
  heatingRegPID.SetSampleTime(10000);
  
  coolingRegPID.SetOutputLimits(0, getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW));          //what is heating regulation time windows ???
  //turn the PID on, cf. PID library
  coolingRegPID.SetMode(AUTOMATIC);                 
  //set PID sampling time to 10000ms                   //possibly set a timer condition with a nilsleep instead
  coolingRegPID.SetSampleTime(10000);
  
  
  heatingRegWindowStartTime = millis();
  // heatingRegSetpoint = getParameter(PARAM_TEMP_MAX);
}

void mode_auto_switch(){
     //auto mode switch
    /*if((getParameter(PARAM_TARGET_LIQUID_TEMP)<(getParameter(PARAM_AMBIENT_TEMP)+300))
    &&(getParameter(PARAM_TARGET_LIQUID_TEMP)>(getParameter(PARAM_AMBIENT_TEMP)-300)))
      //set around ambient mode
      setAndSaveParameter(PARAM_PID_STATUS,2);*/
      
    /*else*/ if(getParameter(PARAM_TARGET_LIQUID_TEMP)>=(getParameter(PARAM_AMBIENT_TEMP)/*+300*/))
      //set heating mode
      setAndSaveParameter(PARAM_PID_STATUS,0);
      
    else if((getParameter(PARAM_TARGET_LIQUID_TEMP)<(getParameter(PARAM_AMBIENT_TEMP)/*-300*/)))
      //set cooling mode
      setAndSaveParameter(PARAM_PID_STATUS,1);
}

#endif
#endif
