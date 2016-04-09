
#include <NilRTOS.h>

// Library that allows to start the watch dog allowing automatic reboot in case of crash
// The lowest priority thread should take care of the watch dog
#include <avr/wdt.h>

// http://www.arduino.cc/playground/Code/Time
#include <Time.h>



#define THR_MONITORING     14  // will also take care of the watch dog




void setup() {
  delay(2000);

  setupLogger();
  setupDebugger();
  setupParameters();
 
Serial1.begin(57600);
  nilSysBegin();
}


void loop() {
}












