#ifdef STEPPER

/****************************
 *  THREAD STEPPER MOTOR
 *  This is the thread controlling the motor. It should have a high priority 
 *  as it is called very often and is short. It controls the sequence with the pin PWM and IO
 *  of the port.
 *  The sequence for turning the motor turn is :
 *  RED-GREEN-BLUE-BLACK => win turn clockwise (top view) where :
 *  RED = {PWM=LOW, IO=LOW}
 *  BLUE = {PWM=LOW, IO=HIGH}
 *  BLACK = {PWM=HIGH, IO=HIGH}
 *  RED = {PWM=HIGH, IO=LOW}
 ******************************/

//We define here the number of step executed during every call to the thread
// Maximum 65535 !!!!
#define NB_STEP_CALL  2400


//Prototypes
void executeStep(uint16_t numberSteps, boolean forward, byte port1, byte port2);


NIL_WORKING_AREA(waThreadStepper, 0);
NIL_THREAD(ThreadStepper, arg) {
  nilThdSleepMilliseconds(4000);
  byte STEPPER_TAB[]=STEPPER;
  boolean forward = true;
  uint8_t count = 0;
  for (byte i=0; i<sizeof(STEPPER_TAB); i++) {
    pinMode(STEPPER_TAB[i], OUTPUT);    
  }
  while (true) {
    //first a check is performed on the motor status

    executeStep(NB_STEP_CALL, forward, STEPPER_TAB[1],STEPPER_TAB[0]);
    forward = !forward;

    nilThdSleepMilliseconds(1000);
  }
}

void executeStep(uint16_t numberSteps, boolean forward, byte port1, byte port2) {
  uint8_t counter=0;
  while (numberSteps>0) {
    if(getParameterBit(PARAM_STATUS, FLAG_STEPPER_CONTROL)==0) {
      return;
    }
    numberSteps--;
    if (forward) {
      counter++;
    }
    else { 
      counter--;
    }

    switch (counter % 4) {
    case 0:
      //This is RED & BLUE
      digitalWrite(port1, LOW);
      digitalWrite(port2,LOW);
      break;
    case 1:   // 1 or 2
      //This is BLUE
      digitalWrite(port1, LOW);
      digitalWrite(port2,HIGH);
      break;
    case 2:   // 2 or 3
      //This is Black
      digitalWrite(port1, HIGH);
      digitalWrite(port2,HIGH);
      break;
    case 3:   // 3 or 1
      //This is Green
      digitalWrite(port1, HIGH);
      digitalWrite(port2,LOW);
      break;
    }
    nilThdSleepMilliseconds(24-(getParameter(PARAM_STEPPER_SPEED)%22));
  } 

}

#endif



