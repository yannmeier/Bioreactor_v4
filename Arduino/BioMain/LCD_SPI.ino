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

/**
 * Check if parameter data received from LCD is valid.
 */
boolean checkParameterData(int parameter, int value){
  // TODO
  
  // check if paramater is in modifiable parameter list
  
  // check parameter values for certain parameters
  
  return true;
}

byte buff[2*MAX_PARAM];
/***********************************************
        SPI Slave thread for SPI
************************************************/
NIL_WORKING_AREA(waThreadLCD, 24);
NIL_THREAD(ThreadLCD, arg) {

  nilThdSleepMilliseconds(1000);

  byte in_byte;                        // incoming byte from LCD
  boolean getting_bytes;               // flag indicating if byte is expected or not
  byte parameter;                      // parsed parameter number 
  int value;                           // parsed parameter value
  byte pos;                            // number of bytes read after initiating byte
  
  while(true) {

  in_byte = 0;
  getting_bytes = false;
  parameter = 0;
  value = 0;
  pos = 0;

  #ifdef FLASH_SELECT
    //Last_Log_To_SPI_buff(buff);             //Load last log in flash 
  #endif
  Last_Params_To_SPI_buff(buff);              //Load Last Parameters Into SPI buffer
  digitalWrite(LCD_SELECT, LOW);              //enable LCD_SS
  for(int i=0; i<sizeof(buff); i++){

    /////////////////////////
    // Read and send bytes
    /////////////////////////

    in_byte = SPI.transfer(char(buff[i]));    // transfer and get bytes

    /////////////////////////
    // Parse read bytes
    /////////////////////////
    
    if (in_byte && !getting_bytes){           // when the intiating byte (1) is received...
      getting_bytes = true;                   // set flag to read setting data
      #ifdef DEBUG_LCD
      Serial.println(F("Getting bytes."));    
      #endif
      delayMicroseconds(20);                  // longer delay when data is expected in return
    }else if (getting_bytes){
      switch(pos){
        case 0:                               // expected parameter byte
          parameter = in_byte;  
          #ifdef DEBUG_LCD          
          Serial.print(F("Parameter: "));
          Serial.println(parameter);  
          #endif
        break;
        case 1:                               // expected first byte of value 
          value = (int)(in_byte<<8);
        break;
        case 2:
          value = value^((int)(in_byte));             // expected second byte of value
       
          if (checkParameterData(parameter, value)){  // if data is valid, change parameter 
         // setAndSaveParameter(parameter, value);    // !!! uncomment only for real usage of the bioreactor !!!
            setParameter(parameter, value);           // use this for development purposes 
          }
          #ifdef DEBUG_LCD
          Serial.print(F("Value: ")); 
          Serial.println(value);  
          #endif
          getting_bytes = false;              // when all setting data has been transmitted change flag to stop reading data
        break;
        default:
          getting_bytes = false;              // in case of an unexpected initiating byte (1) reset flag to stop reading data
      }
      pos++;
      delayMicroseconds(20);
    }else
      delayMicroseconds(10);              //correct transmission errors (normal usage: 10 us, get data from LCD: 20 us)                                   // TODO: make delay dynamic wether bytes are awaited or not
  }

  SPI.transfer('\n');                  //double return at end of com  
  SPI.transfer('\n');
  digitalWrite(LCD_SELECT, HIGH);      // disable LCD_SS
  nilThdSleepMilliseconds(100);
  }
}


#endif

