#ifdef TEMP_PID
#ifdef TEMPERATURE_CTRL

#include <PID_v1.h>


#define SAFETY_TEMP      5500
#define TEMP_MAX_HARD    7000
void pid_ctrl();
void heatingSetup();

double heatingRegInput;
double heatingRegOutput;
double heatingRegSetpoint;
//Specify the heating regulation links and initial tuning parameters
PID heatingRegPID(&heatingRegInput, &heatingRegOutput, &heatingRegSetpoint, 1,0.0002, 5, DIRECT);

NIL_WORKING_AREA(waThread_PID, 128); //tune the allocated mem (here extra is provided)
NIL_THREAD(Thread_PID, arg) 
{
  nilThdSleepMilliseconds(5000); 
  pinMode(TEMP_PID, OUTPUT);
  heatingSetup();
  
  while(TRUE){
    pid_ctrl();
    nilThdSleepMilliseconds(1000);  //refresh every 500ms --> the faster the better the control
  }
}


/*Temperature PID Control addressing relay*/

void pid_ctrl()
{
  uint16_t target=(uint16_t)(getParameter(PARAM_TEMP_TARGET)) ;
  if(target> TEMP_MAX_HARD || target> getParameter(PARAM_TEMP_MAX)){
    //some error event for setting the temp to high here
    setAndSaveParameter(PARAM_TEMP_TARGET,min(TEMP_MAX_HARD,getParameter(PARAM_TEMP_MAX)));
    target=min(TEMP_MAX_HARD,getParameter(PARAM_TEMP_MAX));
  }
  heatingRegInput = getParameter(PARAM_TEMP_LIQ);
  heatingRegSetpoint = target;
  heatingRegPID.Compute();                                   // the computation takes only 30ms!
  if((getParameter(PARAM_TEMP_PCB)<SAFETY_TEMP)
      && (getParameter(PARAM_TEMP_PCB) != 0xFF)
      && (getParameter(PARAM_TEMP_LIQ)   !=0xFF)){
       analogWrite(TEMP_PID, heatingRegOutput);
   }
   else analogWrite(TEMP_PID,0);
}


// see the rest of oliver's code for sanity checks
void heatingSetup()
{
  //tell the PID to range between 0 and the full window size
  heatingRegPID.SetOutputLimits(0, 200); //200 is ~75% of max PWM --> Limits max avg power to ~7.5A
  heatingRegPID.SetMode(AUTOMATIC);      //turn the PID on, cf. PID library             
  heatingRegPID.SetSampleTime(950);      //set PID sampling time to 450ms
}

#endif
#endif
