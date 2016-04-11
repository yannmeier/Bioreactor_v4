#ifdef LCD_SELECT

/************************************
       LCD Control Functions
************************************/
void setupLCD(){
  pinMode(LCD_SELECT,OUTPUT); digitalWrite(LCD_SELECT, HIGH);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  #ifndef FLASH_SELECT
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  #endif
}


/***********************************************
        SPI Slave thread for SPI
************************************************/
NIL_WORKING_AREA(waThreadLCD, 24); // minimum 128
NIL_THREAD(ThreadLCD, arg) {

  nilThdSleepMilliseconds(1000);
  while(true) {

  char c;

  // enable Slave Select
  digitalWrite(LCD_SELECT, LOW);    // SS is pin 10
  // send test string
  for (const char * p = "Hello, world!\n" ; c = *p; p++)
  SPI.transfer (c);
  // disable Slave Select
  digitalWrite(LCD_SELECT, HIGH);

  nilThdSleepMilliseconds(5000);
  }
 
}


#endif

