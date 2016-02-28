//utilities for ph control when using a Table data type
#if defined(PH_TABLE) && defined(PH_CTRL)

/*************************************
      Calibration function
*************************************/

  void get_calibration_parameters(int *ph){
    
    //To get the correct parameters
    setAndSaveParameter(PARAM_PH_FACTOR_A, 1000);//
    setAndSaveParameter(PARAM_PH_FACTOR_B, 0);
    int i=0;
    setAndSaveParameter(PARAM_PH_STATE, 0); //pH control back to PAUSE, avoid bugs
    if(getpH()==3000){
      //Serial.println("pH?");
      nilThdSleepMilliseconds(200);
      return;
    }
    
    //pH 4
    Serial.print(F("Cln"));
    delay_interactive(30);
    nilThdSleepMilliseconds(200);
    Serial.print(F("4"));
    nilThdSleepMilliseconds(200);
    delay_interactive(20);
    ph[PH4] = getpH();  
    for(i=0;i<40;i++){
      ph[PH4]= 0.8*ph[PH4] + 0.2*getpH();     
      Serial.print(F("o")); 
      nilThdSleepMilliseconds(1000);
    }
    Serial.println(F(".")); 
    setParameter(PARAM_REF_PH4, ph[PH4]);

    //pH 7    
    Serial.print(F("Cln"));
    delay_interactive(30);
    nilThdSleepMilliseconds(200);
    Serial.print(F("7"));
    nilThdSleepMilliseconds(200);
    delay_interactive(20);
    ph[PH7] = getpH();  
    for(i=0;i<40;i++){
      ph[PH7] = 0.8*ph[PH7] + 0.2*getpH();     
      Serial.print(F("o")); 
      nilThdSleepMilliseconds(1000);
    }
    Serial.println(F("")); 
    setParameter(PARAM_REF_PH7, ph[PH7]);
    Serial.println(F("OK"));
  }
    
  void delay_interactive(int t){
    int i=0;
    for(i=0;i<t;i++){
      nilThdSleepMilliseconds(1000);
      Serial.print(F(""));
    }
  }

  
void control_ph(unsigned long *tap, int *ph){
  switch(tap[TAP_STATE]){   
    case TAP_STATE_CLOSED:
      if(!tap_Ready(tap[TIMECLOSED]))
        break;//DO NOTHING, wait for adjustment (mix the water..)        
      if(ph[VALUE] > getParameter(PARAM_TARGET_PH)+getParameter(PARAM_PH_TOLERANCE))
        tap[TAP_STATE] = TAP_STATE_ADDING_ACID;        
      else if(ph[VALUE] < getParameter(PARAM_TARGET_PH)-getParameter(PARAM_PH_TOLERANCE))
        tap[TAP_STATE] = TAP_STATE_ADDING_BASE;
      else
        tap[TAP_STATE] = TAP_STATE_CLOSED;       
      break;

    case TAP_STATE_ADDING_BASE:
      add_base(tap);
      break;

    case TAP_STATE_ADDING_ACID:
      add_acid(tap);
      break;
  }
}  
  
  
/*************************
      Dependencies
**************************/


 //ACID TAP
  void add_acid(unsigned long *tap){
    if(tap_Ready(tap[TIMECLOSED])){
      #ifdef DEBUG_PH
        Serial.println(F("+acid"));
      #endif
      open_tap(TAP_ACID);
      nilThdSleepMilliseconds(pid_controler_ph());
      close_tap(TAP_ACID);
      tap[TAP_STATE] = TAP_STATE_CLOSED;
      tap[TIMECLOSED] =millis();
    }
    else
      close_tap(TAP_ACID);
  }

  //BASE TAP
  void add_base(unsigned long *tap){
    if(tap_Ready(tap[TIMECLOSED])){
      #ifdef DEBUG_PH
        Serial.println(F("+ base"));
      #endif
      open_tap(TAP_BASE);
      nilThdSleepMilliseconds(pid_controler_ph());
      close_tap(TAP_BASE);
      tap[TAP_STATE]= TAP_STATE_CLOSED;
      tap[TIMECLOSED] = millis();
    }
    else
      close_tap(TAP_BASE);
  }
#endif
