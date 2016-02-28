#ifdef TEMP_PID_COOLD

NIL_WORKING_AREA(waThread_Stabilized, 32); // minimum of 16 The momory change with time
NIL_THREAD(Thread_Stabilized, arg) 
{
  //extras for regulation around ambient temp case
  nilThdSleepMilliseconds(5000); 
  int duty_cycle=1000;
  int duty_percent=100;
 
 while(true){
  
  int stabilization_temp=getParameter(PARAM_AMBIENT_TEMP)+500;
  long int avg_stabilized=0;
  long int counter=0;
  long int start_time=millis();
  
  if(getParameter(PARAM_PID_STATUS==2)){
   
   while(millis()<(start_time+duty_cycle)){   
     
     avg_stabilized=avg_stabilized+getParameter(PARAM_TEMP_PLATE);
     counter++;
     
     if(millis()<(start_time+(duty_percent*duty_cycle/1000)))
       digitalWrite(TEMP_PID_HOT, HIGH); 
     else
       digitalWrite(TEMP_PID_HOT, LOW);
     nilThdSleepMilliseconds(20);
   }
   
    avg_stabilized=avg_stabilized/counter;
    if(avg_stabilized<stabilization_temp)
      duty_percent=min((duty_percent+1),1000);
    else
      duty_percent=max(0,duty_percent-1);
 
  }
  
  else 
    nilThdSleepMilliseconds(500);

} 
  
}

#endif
