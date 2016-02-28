#define ERROR_VALUE  -32768


// Definition of all events to be logged
#define EVENT_ARDUINO_BOOT           1
#define EVENT_ARDUINO_SET_SAFE       2
#define EVENT_RESET_ETHERNET         3

#define EVENT_PUMPING_FILLING_START    10
#define EVENT_PUMPING_FILLING_STOP     11
#define EVENT_PUMPING_FILLING_FAILURE  12

#define EVENT_PUMPING_EMPTYING_START   13
#define EVENT_PUMPING_EMPTYING_STOP    14
#define EVENT_PUMPING_EMPTYING_FAILURE 15

#define EVENT_PUMPING_WAITING          16

#define EVENT_MOTOR_START            20
#define EVENT_MOTOR_STOP             21


#define EVENT_TEMP_LIQ_FAILED        50
#define EVENT_TEMP_LIQ_RECOVER       51
#define EVENT_TEMP_PLATE_FAILED      52
#define EVENT_TEMP_PLATE_RECOVER     53
#define EVENT_TEMP_STEPPER_FAILED    54
#define EVENT_TEMP_STEPPER_RECOVER   55
#define EVENT_TEMP_SAMPLE_FAILED    54
#define EVENT_TEMP_SAMPLE_RECOVER   55


#define EVENT_WEIGHT_FAILURE           129
#define EVENT_WEIGHT_BACK_TO_NORMAL    130

#define EVENT_ERROR_NOT_FOUND_ENTRY_N  150




void resetParameters() {

#ifdef STEPPER
  setAndSaveParameter(PARAM_STEPPER_SPEED, 15);
#endif   
  
#ifdef     TEMPERATURE_CTRL
  setAndSaveParameter(PARAM_TEMP_LIQ, ERROR_VALUE);
  setAndSaveParameter(PARAM_TEMP_PLATE, ERROR_VALUE);
  setAndSaveParameter(PARAM_TARGET_LIQUID_TEMP, 3000);
  setAndSaveParameter(PARAM_TEMP_MAX, 0);
  #ifdef TEMP_PID
  setAndSaveParameter(PARAM_HEATING_REGULATION_TIME_WINDOW, 5000);
  setAndSaveParameter(PARAM_MIN_TEMPERATURE, 1000);  // not used but could be used for safety
  setAndSaveParameter(PARAM_MAX_TEMPERATURE, 4000);  // not used but could be used for safety  
  #endif
#endif

#ifdef FOOD_CTRL 
  setAndSaveParameter(PARAM_WEIGHT_MIN, 32767);
  setAndSaveParameter(PARAM_WEIGHT_MAX, -32767);
  setAndSaveParameter(PARAM_SEDIMENTATION_TIME, 30);
  setAndSaveParameter(PARAM_MIN_FILLED_TIME, 30);
  setAndSaveParameter(PARAM_WEIGHT_STATUS, 0);
  setAndSaveParameter(PARAM_MIN_ABSOLUTE_WEIGHT, 170);
  setAndSaveParameter(PARAM_MAX_ABSOLUTE_WEIGHT, 300);
  setAndSaveParameter(PARAM_WEIGHT_FACTOR, 1000);
  setAndSaveParameter(PARAM_WEIGHT_OFFSET, 0);
#endif

#ifdef PH_CTRL
  setAndSaveParameter(PARAM_TARGET_PH, 700);
  setAndSaveParameter(PARAM_PH_FACTOR_A, -6685);
  setAndSaveParameter(PARAM_PH_FACTOR_B, -2170);
  setAndSaveParameter(PARAM_PH_STATE, 0);
  setAndSaveParameter(PARAM_PH_ADJUST_DELAY, 10);
  setAndSaveParameter(PARAM_PH_OPENING_TIME, 1);
  setAndSaveParameter(PARAM_PH_TOLERANCE, 10);
  setAndSaveParameter(PARAM_REF_PH4, 4000);
  setAndSaveParameter(PARAM_REF_PH7, 7000);
  setAndSaveParameter(PARAM_REF_PH10, 10000);

#endif

#ifdef TAP_GAS1
  setAndSaveParameter(PARAM_FLUX_GAS1, ERROR_VALUE);
  setAndSaveParameter(PARAM_DESIRED_FLUX_GAS1, ERROR_VALUE);
  setAndSaveParameter(PARAM_ANEMO_OFFSET1,0);
  setAndSaveParameter(PARAM_ANEMO_FACTOR1,100);
#endif
#ifdef TAP_GAS2
  setAndSaveParameter(PARAM_FLUX_GAS2, ERROR_VALUE);
  setAndSaveParameter(PARAM_DESIRED_FLUX_GAS2, ERROR_VALUE);
  setAndSaveParameter(PARAM_ANEMO_OFFSET2,0);
  setAndSaveParameter(PARAM_ANEMO_FACTOR2,100);
#endif
#ifdef TAP_GAS3
  setAndSaveParameter(PARAM_FLUX_GAS3, ERROR_VALUE);
  setAndSaveParameter(PARAM_DESIRED_FLUX_GAS3, ERROR_VALUE);
  setAndSaveParameter(PARAM_ANEMO_OFFSET3,0);
  setAndSaveParameter(PARAM_ANEMO_FACTOR3,100);
#endif
#ifdef TAP_GAS4
  setAndSaveParameter(PARAM_FLUX_GAS4, ERROR_VALUE);
  setAndSaveParameter(PARAM_DESIRED_FLUX_GAS4, ERROR_VALUE);
  setAndSaveParameter(PARAM_ANEMO_OFFSET4,0);
  setAndSaveParameter(PARAM_ANEMO_FACTOR4,100);
#endif
#ifdef GAS_CTRL
  setAndSaveParameter(PARAM_FLUX_TOLERANCE, 100);
  setAndSaveParameter(PARAM_FLUX_TIME_WINDOWS, 10);
#endif

  setAndSaveParameter(PARAM_STATUS, 15); // 0b0000 1111 activate food_control, ph_control, gas_control, stepper_control

}
