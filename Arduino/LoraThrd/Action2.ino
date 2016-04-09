#define THR_ACTION2 1

#ifdef THR_ACTION2

NIL_WORKING_AREA(waThreadAction2, 0);
NIL_THREAD(ThreadAction2, arg) {

  byte turnOn=0;
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  while (TRUE) {
    turnOn=~turnOn;
    digitalWrite(0,turnOn);
    digitalWrite(1,turnOn);
    nilThdSleepMilliseconds(1);
  }


}

#endif














