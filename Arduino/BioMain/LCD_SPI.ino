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

byte buff[64];
/***********************************************
        SPI Slave thread for SPI
************************************************/
NIL_WORKING_AREA(waThreadLCD, 24); // minimum 128
NIL_THREAD(ThreadLCD, arg) {

  nilThdSleepMilliseconds(1000);
  while(true) {
  
  Last_Log_To_SPI_buff(buff);
  // enable Slave Select
  digitalWrite(LCD_SELECT, LOW);    // SS is pin 10
  //Load Last Log Into SPI buffer
  for(int i=0; i<sizeof(buff); i++){
    SPI.transfer(char(buff[i]));
    delayMicroseconds(10); //correct transmission error by adding a small delay before start & stop.
  }
  //double chariot return at end of communication  
  SPI.transfer('\n');
  SPI.transfer('\n');
  // disable Slave Select
  digitalWrite(LCD_SELECT, HIGH);

  nilThdSleepMilliseconds(1000);
  }
 
}


#endif

