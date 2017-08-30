#ifdef STEPPER
/****************************
    THREAD STEPPER MOTOR
    This is the thread controlling the motor. It should have a high priority
    as it is called very often and is short. It controls the sequence with the pin PWM and IO
    of the port.
    The functionning of the stepper is different wether in version 4.5 or in previous versions.
    -----------
    VERSION 4.5
    -----------
    Stepper works with a stepper driver (DRV8811). Each call to function executestep will turn stepper by a bit.
    You should verify which pads are soldered on your board as the frequency of the stepper will vary according to that.
    By default, we will consider both pads to be soldered.
    If both pads are soldered on your board, the stepper will execute 400 steps per rotation (one step corresponds to one toggle of the stepper pin)
    From this knowledge, we can calculate the ratio between our delay between two toggles and our desired speed in RPM
    delay = 6e7 [us/min]/(#[steps/rot] * RPM [rot/min]) = (150000/RPM)[us/step]
    -----------
    VERSION 4.4
    -----------
    The sequence for turning the motor turn is :
    RED-GREEN-BLUE-BLACK => win turn clockwise (top view) where :
    RED = {PWM=LOW, IO=LOW}
    BLUE = {PWM=LOW, IO=HIGH}
    BLACK = {PWM=HIGH, IO=HIGH}
    RED = {PWM=HIGH, IO=LOW}
 ******************************/

//We define here the number of step executed during every call to the thread
//#define NB_STEP_CALL  10000 // Maximum 65535 !!!!

//We define a maximal and a minimal speed (in RPM) for safety and stability reasons
#ifndef BEFORE_45
#define MIN_STEPPER_SPEED 5   // RPM
#define MAX_STEPPER_SPEED 200 // RPM
#define RPM_TO_STEP 150000    // This value is valid only if both pads on the board have been short-circuited
// If the pads have not both been short-ciruicted, see file TestStepperDRV8811.ino for values
#endif

byte STEPPER_TAB[] = STEPPER;

//------------------------------------------------------------------------------------------------------------------//

//--------------- STOP STEPPER ---------------//

void stopStepper() {
#ifdef BEFORE_45
  PORTB &= ~(STEPPER_TAB[0] | STEPPER_TAB[1]);
  PORTF &= ~(STEPPER_TAB[2] | STEPPER_TAB[3]);
#else
  PORTB &= ~(STEPPER_TAB[0] | STEPPER_TAB[1]);
#endif
}

//------------------------------------------------------------------------------------------------------------------//

//--------------- EXECUTE STEP ---------------//

// Current version: V4.5, see lower

/*
   Before V4.3
*/

#ifdef BEFORE_43

void executeStep(uint16_t numberSteps, boolean forward, byte port1, byte port2) {
  uint8_t counter = 0;
  while (numberSteps > 0) {

    if (isStepperStopped()) return;

    numberSteps--;
    if (forward) counter++;
    else counter--;

    if ((getParameter(PARAM_STEPPER_SPEED) % 101) != 0) {
      switch (counter % 4) {
        case 0:
          //This is RED & BLUE
          digitalWrite(port1, LOW);
          digitalWrite(port2, LOW);
          break;
        case 1:   // 1 or 2
          //This is BLUE
          digitalWrite(port1, LOW);
          digitalWrite(port2, HIGH);
          break;
        case 2:   // 2 or 3
          //This is Black
          digitalWrite(port1, HIGH);
          digitalWrite(port2, HIGH);
          break;
        case 3:   // 3 or 1
          //This is Green
          digitalWrite(port1, HIGH);
          digitalWrite(port2, LOW);
          break;
      }
      nilThdSleepMilliseconds(1 + (10 - getParameter(PARAM_STEPPER_SPEED) % 11));
    } else  nilThdSleepMilliseconds(100);
  }
}

/*
   Before V4.5
*/

#elif defined(BEFORE_45)
void executeStep(uint16_t numberSteps, boolean forward) {
  DDRB |= (STEPPER_TAB[0] | STEPPER_TAB[1]) ;
  DDRF |= (STEPPER_TAB[2] | STEPPER_TAB[3]) ;
  uint8_t counter = 0;
  while (numberSteps > 0) {

    if (isStepperStopped()) return;

    numberSteps--;
    if (forward) counter++;
    else counter--;

    if ((getParameter(PARAM_STEPPER_SPEED) % 11) != 0) {
      switch (counter % 4) {
        case 0:
          PORTB |= STEPPER_TAB[0];
          break;
        case 1:
          PORTB |= STEPPER_TAB[1];   // 1 or 2
          break;
        case 3:   // 2 or 3
          PORTF |= STEPPER_TAB[2];
          break;
        case 2:
          PORTF |= STEPPER_TAB[3];
          break;
      }

      nilThdSleepMilliseconds(11 - getParameter(PARAM_STEPPER_SPEED) % 11);

      stopStepper();

    } else  nilThdSleepMilliseconds(100);
  }
}

/*
   Version 4.5
*/

#else
void executeStep(uint16_t numberSteps, boolean forward) {
  DDRB |= (STEPPER_TAB[0] | STEPPER_TAB[1]) ;  // Set pins 8 and 9 as output

  while (numberSteps > 0) {
    if (isStepperStopped()) ret
    urn;
    // Stepper direction
    if (forward) {
      PORTB &= ~STEPPER_TAB[0]; // Turns pin 8 as LOW
    } else {
      PORTB |= STEPPER_TAB[0]; // Turns pin 8 as HIGH
    }

    // Calculate delay

    // The stepper motor recquires 400 steps in order to do a full rotation
    // Ratio to convert the amount of rotations per minute into the interval between each step
    // = 6e4 [ms/min] / 400 [step/rotation] --> divided by RPM --> [ms/step]

    // PARAM_STEPPER_SPEED must now be given in RPM
    if (getParameter(PARAM_STEPPER_SPEED) > MAX_STEPPER_SPEED) {
      setParameter(PARAM_STEPPER_SPEED, MAX_STEPPER_SPEED); // If the given stepper speed is higher than the maximum, the stepper will run at max authorized speed
    }
    if (getParameter(PARAM_STEPPER_SPEED) < MIN_STEPPER_SPEED && getParameter(PARAM_STEPPER_SPEED) > 0) {
      setParameter(PARAM_STEPPER_SPEED, 0); // If the given stepper speed is lower than the minimum, the stepper will stop
    }

    uint32_t delayPerStep = RPM_TO_STEP / getParameter(PARAM_STEPPER_SPEED); // delay to apply between each step

    // Execute step
    numberSteps--;
    if (getParameter(PARAM_STEPPER_SPEED) >= MIN_STEPPER_SPEED) {
      PORTB ^= STEPPER_TAB[1];
      nilThdSleepMicroseconds(delayPerStep);
      stopStepper();
    } else {
      nilThdSleepMilliseconds(100);
    }
  }
}
#endif

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
  stopStepper();
  DDRB |= (STEPPER_TAB[0] | STEPPER_TAB[1]) ;
#endif



  while (true) {
    //first a check is performed on the motor status
#ifdef BEFORE_43
    executeStep((uint16_t)getParameter(PARAM_STEPPER_STEPS), forward, STEPPER_TAB[1], STEPPER_TAB[0]);
#else
    executeStep((uint16_t)getParameter(PARAM_STEPPER_STEPS), forward);
#endif
    forward = !forward;
    nilThdSleepMilliseconds(1000);
  }
}





#endif



