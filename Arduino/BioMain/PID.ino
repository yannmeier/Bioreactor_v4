#ifdef TEMP_PID
#ifdef TEMPERATURE_CTRL

#include <PID_v1.h>


#define SAFETY_TEMP 60000
void pid_ctrl();
void heatingSetup();


double heatingRegInput;
double heatingRegOutput;
double heatingRegSetpoint;
unsigned long heatingRegWindowStartTime;
//Specify the heating regulation links and initial tuning parameters //Kp=100; Ti=0.2; Td=5 are initial testing param.
//PID object definition can be found in PID library (to include for compilation).
PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 7000,15,300, DIRECT); //with one 4.7Ohms resistor (here 5.4)

NIL_WORKING_AREA(waThread_PID, 24); // minimum of 16 The momory change with time
NIL_THREAD(Thread_PID, arg) 
{
  nilThdSleepMilliseconds(5000); 
  pinMode(TEMP_PID, OUTPUT);
  //Todo : update heatingSetup when a parameter is changed
  heatingSetup();
  
  while(TRUE){
    pid_ctrl();
    nilThdSleepMilliseconds(200);  //refresh every 200ms --> the faster the better the control
  }
}


/*Temperature PID Control addressing relay*/

void pid_ctrl()
{
  float exactPresentTime;
  heatingRegInput = getParameter(PARAM_TEMP_LIQ);
  heatingRegSetpoint = getParameter(PARAM_TEMP_TARGET);
  heatingRegPID.Compute();                                   // the computation takes only 30ms!
  // turn the output pin on/off based on pid output
  exactPresentTime = millis();
  if (exactPresentTime - heatingRegWindowStartTime > getParameter(PARAM_TEMP_REG_TIME)) { 
    //time to shift the Relay Window
    heatingRegWindowStartTime += getParameter(PARAM_TEMP_REG_TIME);
  }
  
  if((heatingRegOutput > exactPresentTime - heatingRegWindowStartTime) 
    && (getParameter(PARAM_TEMP_PCB)<SAFETY_TEMP) 
    && (getParameter(PARAM_TEMP_PCB) != 0xFF)
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
  heatingRegPID.SetOutputLimits(0, getParameter(PARAM_TEMP_REG_TIME));          //what is heating regulation time windows ???
  //turn the PID on, cf. PID library
  heatingRegPID.SetMode(AUTOMATIC);                 
  //set PID sampling time to 10000ms                   //possibly set a timer condition with a nilsleep instead
  heatingRegPID.SetSampleTime(10000);
  heatingRegWindowStartTime = millis();
  // heatingRegSetpoint = getParameter(PARAM_TEMP_MAX);
}

#endif
#endif
