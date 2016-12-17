/**************
 * LIBRAIRIES
 **************/
//MultiThread
#include <NilRTOS.h>
//Lib to access memory with SPI
#include <SPI.h>
// Library that allows to start the watch dog
#include <avr/wdt.h>
// http://www.arduino.cc/playground/Code/Time
// We need to deal with EPOCH
#include <Time.h>



/*********
 * SETUP
 *********/

void setup() {
  delay(1000);
  Serial.begin(9600);
  delay(1000);
  setupParameters(); 

  #ifdef FLASH_SELECT 
    pinMode(FLASH_SELECT,OUTPUT);
  setupMemory();
  recoverLastEntryN();
  loadLastEntryToParameters();   //get back the previous config  
  #endif

  #ifdef LCD_SELECT              //disable SPI modules 
    setupLCD();
  #endif
  
  setSafeConditions(false);
  nilSysBegin();
}

void loop() {

}


bool lockTimeCriticalZone=false;
void protectThread() {
   while(lockTimeCriticalZone) {
    nilThdSleepMilliseconds(5);
   }
   lockTimeCriticalZone=true;
}
void unprotectThread() {
	lockTimeCriticalZone=false;
}


