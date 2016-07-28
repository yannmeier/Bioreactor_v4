#ifdef LCD_SELECT

/************************************
       LCD Control Functions
************************************/
void setupLCD(){
  pinMode(LCD_SELECT,OUTPUT); 
  digitalWrite(LCD_SELECT, HIGH);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  #ifndef FLASH_SELECT
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  #endif
}

void Last_Params_To_SPI_buff(byte* buff) { 
  for(byte i = 0; i < MAX_PARAM; i++) {
    buff[2*i]=(byte)((getParameter(i)>>8)&(0x00FF));
    buff[2*i+1]=(byte)((getParameter(i))&(0x00FF));
  }
}

byte buff[2*MAX_PARAM];
/***********************************************
        SPI Slave thread for SPI
************************************************/
NIL_WORKING_AREA(waThreadLCD, 24);
NIL_THREAD(ThreadLCD, arg) {

  nilThdSleepMilliseconds(1000);

  byte in_byte = 0;                        // incoming byte from LCD
  
  while(true) {
  #ifdef FLASH_SELECT
    //Last_Log_To_SPI_buff(buff);      //Load last log in flash 
  #endif
  Last_Params_To_SPI_buff(buff);       //Load Last Parameters Into SPI buffer
  digitalWrite(LCD_SELECT, LOW);       //enable LCD_SS
  for(int i=0; i<sizeof(buff); i++){ 
    in_byte = SPI.transfer(char(buff[i]));
    delayMicroseconds(20);              //correct transmission errors (normal usage: 5 us, get data from LCD: 20 us)
                                        // TODO: make delay dynamic wether bytes are awaited or not
                                        
    if (in_byte){                      // if the LCD starts sending bytes...
                                       // TODO: if the communication is started, read SPI_OUT_BUF_SIZE bytes.  
      Serial.print(F("Incoming byte: "));
      Serial.println(in_byte);
    }
  }
  SPI.transfer('\n');                  //double return at end of com  
  SPI.transfer('\n');
  digitalWrite(LCD_SELECT, HIGH);      // disable LCD_SS
  nilThdSleepMilliseconds(100);
  }
}


#endif

