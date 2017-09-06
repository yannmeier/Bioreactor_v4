#ifdef STEPPER
/****************************
    THREAD STEPPER MOTOR

    The last verison use a stepper driver and therefore we may simply use a Timer to generate
    an impulsion at a very precise time frame.

 ******************************/

#include <TimerOne.h>

#define MIN_STEPPER_SPEED 5   // RPM
#define MAX_STEPPER_SPEED 200 // RPM
#define RPM_TO_STEP 150000    // This value is valid only if both pads on the board have been short-circuited

byte STEPPER_TAB[] = STEPPER;

//------------------------------------------------------------------------------------------------------------------//

//--------------- STOP STEPPER ---------------//

void stopStepper() {
  Timer1.stop();
}


//------------------------------------------------------------------------------------------------------------------//

//--------------- IS STEPPER STOPPED ---------------//

boolean isStepperStopped() {
  if (! isRunning(FLAG_STEPPER_CONTROL) || ! isEnabled(FLAG_STEPPER_CONTROL)) { // PID is disabled
    stopStepper();
    return true;
  }

  if (isError()) { // any error we should stop heating !
    stopStepper();
    return true;
  }

  if (isRunning(FLAG_RELAY_EMPTYING) || isRunning(FLAG_SEDIMENTATION)) {
    stopStepper();
    return true;
  }
  return false;
}

//--------------- STEPPER THREAD ---------------//

NIL_WORKING_AREA(waThreadStepper, 16);
NIL_THREAD(ThreadStepper, arg) {
  nilThdSleepMilliseconds(4000);
  boolean forward = true;
  uint8_t count = 0;


#ifdef BEFORE_43
  for (byte i = 0; i < sizeof(STEPPER_TAB); i++) {
    pinMode(STEPPER_TAB[i], OUTPUT);
  }
#elif defined(BEFORE_45)
  stopStepper();
  DDRB |= (STEPPER_TAB[0] | STEPPER_TAB[1]) ;
  DDRF |= (STEPPER_TAB[2] | STEPPER_TAB[3]) ;
#else

  pinMode(STEPPER_TAB[0], OUTPUT);
  pinMode(STEPPER_TAB[1], OUTPUT);
  Timer1.initialize(5000);  // 5000ms  = 40 Hz
  stopStepper();
#endif



  while (true) {
    //first a check is performed on the motor status

    if (forward) {
      digitalWrite(STEPPER_TAB[0], HIGH);
    } else {
      digitalWrite(STEPPER_TAB[0], LOW);
    }
    for (int i = 0; i < getParameter(PARAM_STEPPER_SECONDS); i++) {
      if (isStepperStopped()) break;
      Timer1.pwm(STEPPER_TAB[1], 512, (50 * 60 / getParameter(PARAM_STEPPER_SPEED)) * 100); // second parameter is duty from 0 to 1023
      Timer1.start();
      nilThdSleepMilliseconds(1000);
    }
    stopStepper();

    forward = !forward;
    nilThdSleepMilliseconds(1000);
  }
}

#endif



