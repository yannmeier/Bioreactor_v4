#include <TimerOne.h>

const int pin = 9;

void setup(void)
{
  Timer1.initialize(5000);  // 5000ms  = 40 Hz
  Timer1.pwm(pin, 511);
}

void loop(void) {
  byte rpm = 120;
  Timer1.setPeriod((50 * 60 / rpm) * 100); // 5000 is 1 rotation per seconds
delay(100000);
}



