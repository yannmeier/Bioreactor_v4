#ifdef TEMP_PID
#ifdef TEMPERATURE_CTRL

#include <PID_v1.h>


void pid_ctrl();
void heatingSetup();


double heatingRegInput;
double heatingRegOutput;
double heatingRegSetpoint;
unsigned long heatingRegWindowStartTime;
//Specify the heating regulation links and initial tuning parameters //Kp=100; Ti=0.2; Td=5 are initial testing param.
//PID object definition can be found in PID library (to include for compilation).
//PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 7000,15,300, DIRECT); //with one 4.7Ohms resistor
PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 10000,15,300, DIRECT);  //with two series resistors of 4.7Ohms

NIL_WORKING_AREA(waThread_PID, 24); // minimum of 16 The momory change with time
NIL_THREAD(Thread_PID, arg) 
{
  

  nilThdSleepMilliseconds(5000); 
  pinMode(TEMP_PID, OUTPUT);
  //Todo : update heatingSetup when a parameter is changed
  heatingSetup();
  
  while(TRUE){
    pid_ctrl();
    nilThdSleepMilliseconds(500);  //refresh every 500ms
  }
}


/*Temperature PID Control addressing relay*/

void pid_ctrl()
{
  float exactPresentTime;
  heatingRegInput = getParameter(PARAM_TEMP_LIQ);
  heatingRegSetpoint = getParameter(PARAM_TARGET_LIQUID_TEMP);
  heatingRegPID.Compute();                                   // the computation takes only 30ms!
  // turn the output pin on/off based on pid output
  exactPresentTime = millis();
  if (exactPresentTime - heatingRegWindowStartTime > getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW)) { 
    //time to shift the Relay Window
    heatingRegWindowStartTime += getParameter(PARAM_HEATING_REGULATION_TIME_WINDOW);
  }
  
  if((heatingRegOutput > exactPresentTime - heatingRegWindowStartTime) 
    && (getParameter(PARAM_TEMP_PLATE)<getParameter(PARAM_TEMP_MAX))
    && (getParameter(PARAM_TEMP_PLATE)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_LIQ)<getParameter(PARAM_MAX_TEMPERATURE))
    && (getParameter(PARAM_TEMP_PLATE) != 0xFF)
    && (getParameter(PARAM_TEMP_LIQ)   !=0xFF))
  {
    digitalWrite(TEMP_PID, HIGH); 
  }  
  else 
  {
    digitalWrite(TEMP_PID, LOW); 
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
  heatingRegWindowStartTime = millis();
  // heatingRegSetpoint = getParameter(PARAM_TEMP_MAX);
}

#endif
#endif
