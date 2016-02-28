#ifdef MODE_CALIBRATE

#define PARAM_SELECT_ANEMOM_CHANNEL   13 //select the channel you want to test.

NIL_WORKING_AREA(waThreadCalibration, 80);   
NIL_THREAD(ThreadCalibration, arg) {


  /*********** Gas Calibration ***********/

  const byte param_anem_offset[]={
    #ifdef TAP_GAS1 
    PARAM_ANEMO_OFFSET1,
    #endif
    #ifdef TAP_GAS2
    PARAM_ANEMO_OFFSET2,
    #endif
    #ifdef TAP_GAS3
    PARAM_ANEMO_OFFSET3,
    #endif
    #ifdef TAP_GAS4
    PARAM_ANEMO_OFFSET4
    #endif
  };

  for(int i=0;i<10;i++)
  {
    Serial.print(F("."));
    nilThdSleepMilliseconds(1000);
  }

  Serial.println(F("."));
  Serial.println(F("Calibr Anemo"));
  Serial.println(F("Connect anemo to I2C bus if you want to recalibrate it. If not, disconnect it within 15 seconds!"));
  nilThdSleepMilliseconds(15000);
  if(wireDeviceExists(I2C_FLUX))
  {
    Serial.println(F("Anemometer connected. Calibration starting. Make sure NO gas is flowing through the anemeometer."));
    Serial.println(F("This takes 2 minutes..."));
    nilThdSleepMilliseconds(90000);
    byte nTaps=sizeof(param_flux_gas);
    for(byte i(0);i<nTaps;i++)
    {
      setParameter(param_anem_offset[i], 0); //set all the offsets to 0
    }
    nilThdSleepMilliseconds(10000);
    Serial.println(F("Recording starts. 20 seconds left"));
    
    int offset[nTaps];
    for(byte i(0);i<nTaps;i++)
    {
      offset[i]=10*getParameter(param_flux_gas[i]); // factor 10 to get more precision. is divided by 10 in the end again.
    }
    for(byte i(0);i<20;i++)
    {
      for(byte i(0);i<nTaps;i++)
      {//average the measured value
        offset[i]=0.9*offset[i]+getParameter(param_flux_gas[i]);
      }
      nilThdSleepMilliseconds(1000);
    }

    for(byte i(0);i<nTaps;i++)
    {
      setAndSaveParameter(param_anem_offset[i], (int)(offset[i]/10));// remove factor 10 again.
    }
  }
  else
  {
    Serial.print(F("Anemometer not connected. Continuing with weight calibration."));
  }



  /*********** Weight Calibration ***********/

  unsigned long millisSinceLastEvent=0; // when was the last food cycle
  unsigned long lastCycleMillis=millis(); // when was the last food cycle
  int weightSinceLastEvent=0; //delta m
  int lastCycleWeight=getParameter(PARAM_WEIGHT); //get current weight of bioreactor content
  int measuredGasFlow=getParameter(getParameter(PARAM_SELECT_ANEMOM_CHANNEL)+5);//translate the channel number in the parameter number


  while(true){ 
    nilThdSleepMilliseconds(250);  

    millisSinceLastEvent=millis()-lastCycleMillis;
    lastCycleMillis=millis();
    weightSinceLastEvent=getParameter(PARAM_WEIGHT)-lastCycleWeight;
    lastCycleWeight=getParameter(PARAM_WEIGHT);
    
    Serial.print(lastCycleWeight);
    //Serial.print(weightSinceLastEvent);
    Serial.print(F(","));
    Serial.print(millisSinceLastEvent);
    Serial.print(F(","));
    Serial.println(measuredGasFlow);

    //measure in the "middle"
    measuredGasFlow=getParameter(getParameter(PARAM_SELECT_ANEMOM_CHANNEL)+5);
    nilThdSleepMilliseconds(250);  


  }
}

#endif




















