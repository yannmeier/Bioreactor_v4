// Nice way to make some monitoring about activity. This should be the lower priority process
// If the led is "stable" (blinks 500 times per seconds) it means there are not too
// many activities on the microcontroler

#define MAX_THREADS 10

boolean threads[MAX_THREADS];



NIL_WORKING_AREA(waThreadMonitoring, 0);
NIL_THREAD(ThreadMonitoring, arg) {
  // we should not start the watchdog too quickly ...
  nilThdSleepMilliseconds(10000);

  // we activate the watchdog
  // we need to make a RESET all the time otherwise automatic reboot: wdt_reset();
  wdt_enable(WDTO_8S);


#ifdef MONITORING_LED
  pinMode(MONITORING_LED, OUTPUT);   
#endif
  while (TRUE) {
#ifdef MONITORING_LED

    digitalWrite(MONITORING_LED,HIGH);
    nilThdSleepMilliseconds(500); //was 1000ms init
    digitalWrite(MONITORING_LED,LOW);
    nilThdSleepMilliseconds(500);
#endif
    
    #ifdef TYPE_ETHERNET_GENERAL
      autoreboot++;
    #endif
    
    if (autoreboot<AUTOREBOOT) {
      wdt_reset();
    } 
    else {
      if (autoreboot==AUTOREBOOT) {
        saveParameters();
        setSafeConditions(true);
      }
    }
  }
}



NIL_THREADS_TABLE_BEGIN()

#ifdef THR_ZIGBEE
NIL_THREADS_TABLE_ENTRY(NULL, ThreadZigbee, NULL, waThreadZigbee, sizeof(waThreadZigbee))
#endif

#ifdef  THR_STEPPER
NIL_THREADS_TABLE_ENTRY(NULL, ThreadStepper, NULL, waThreadStepper, sizeof(waThreadStepper))
#endif

#ifdef THR_LINEAR_LOGS
NIL_THREADS_TABLE_ENTRY(NULL, ThreadLogger, NULL, waThreadLogger, sizeof(waThreadLogger))
#endif

#ifdef GAS_CTRL
#ifndef MODE_CALIBRATE
NIL_THREADS_TABLE_ENTRY(NULL, ThreadTap, NULL, waThreadTap, sizeof(waThreadTap))
#endif  
#endif

#ifdef TEMPERATURE_CTRL
NIL_THREADS_TABLE_ENTRY(NULL, ThreadTemp, NULL, waThreadTemp, sizeof(waThreadTemp))  
#ifdef TEMP_PID                      
NIL_THREADS_TABLE_ENTRY(NULL, Thread_PID, NULL, waThread_PID, sizeof(waThread_PID))  
#endif   
#ifdef TEMP_PID_COLD     
/*NIL_THREADS_TABLE_ENTRY(NULL, Thread_Stabilized, NULL, waThread_Stabilized, sizeof(waThread_Stabilized))*/
NIL_THREADS_TABLE_ENTRY(NULL, Thread_HOT_N_COLD, NULL, waThread_HOT_N_COLD, sizeof(waThread_HOT_N_COLD))  
#endif    
#endif

#ifdef FOOD_CTRL
NIL_THREADS_TABLE_ENTRY(NULL, ThreadWeight, NULL, waThreadWeight, sizeof(waThreadWeight))
#endif

#ifdef THR_SERIAL
NIL_THREADS_TABLE_ENTRY(NULL, ThreadSerial, NULL, waThreadSerial, sizeof(waThreadSerial))
#endif

#if defined(GAS_CTRL) || defined(I2C_LCD) || defined(PH_CTRL) || defined(I2C_RELAY_FOOD)
NIL_THREADS_TABLE_ENTRY(NULL, ThreadWire, NULL, waThreadWire, sizeof(waThreadWire))
#endif

#ifdef THR_ETHERNET
NIL_THREADS_TABLE_ENTRY(NULL, ThreadEthernet, NULL, waThreadEthernet, sizeof(waThreadEthernet))
#endif

#ifdef PH_CTRL_OLD
NIL_THREADS_TABLE_ENTRY(NULL, ThreadPH, NULL, waThreadPH, sizeof(waThreadPH))
#endif

#ifdef THR_MONITORING
NIL_THREADS_TABLE_ENTRY(NULL, ThreadMonitoring, NULL, waThreadMonitoring, sizeof(waThreadMonitoring))
#endif

#ifdef MODE_CALIBRATE
NIL_THREADS_TABLE_ENTRY(NULL, ThreadCalibration, NULL, waThreadCalibration, sizeof(waThreadCalibration))
#endif

NIL_THREADS_TABLE_END()
