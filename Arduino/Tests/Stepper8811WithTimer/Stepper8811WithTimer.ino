#include <TimerOne.h>

const int pin = 9;

void setup(void)
{
  Timer1.initialize(5000);  // 5000ms  = 40 Hz
  Timer1.pwm(pin, 511);
}

void loop(void) {
  for (int i = 3000; i < 6001; i = i + 1000) {
    Timer1.setPeriod(4600);
    delay(1000);
    Timer1.stop();
    delay(4000);
  }
}



