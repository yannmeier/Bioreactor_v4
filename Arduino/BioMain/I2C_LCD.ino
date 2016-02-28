 /*****************
     * LCD I2C Module
     *****************/

void updateLcd() {
  
#ifdef I2C_LCD
    if (wireEventStatus%10==0) {
      if (wireDeviceExists(I2C_LCD)) {
        if (! wireFlagStatus(wireFlag32, I2C_LCD)) {
          // we should be able to dynamically change the LCD I2C bus address
          setWireFlag(wireFlag32, I2C_LCD);
          lcd.begin(16,2);
          // Print a message to the LCD.
          lcd.setCursor(0,0);
          lcd.print(F("IO1:"));
          lcd.setCursor(0,1);
          lcd.print(F("IO2:"));
        }
        /* To be replaced with the information we want
         lcd.setCursor(4,0);
         lcd.print(((float)getParameter(PARAM_TEMP1))/100);
         lcd.print(F("C "));
         lcd.setCursor(4,1);
         lcd.print(getParameter(PARAM_DISTANCE));
         lcd.print(F("mm  "));
         
         lcd.setCursor(12,1);
         lcd.print(getParameter(PARAM_IRCODE));
         lcd.print(F("   "));(
         */
      } 
      else {
        clearWireFlag(wireFlag32, I2C_LCD); 
      }
    }

    /*
    if (wireEventStatus%10==5) {
     if (wireDeviceExists(WIRE_LCD_20_4)) {
     if (! wireFlagStatus(wireFlag32, WIRE_LCD_20_4)) {
     // we should be able to dynamically change the LCD I2C bus address
     setWireFlag(wireFlag32, WIRE_LCD_20_4);
     lcd.begin(20, 4);
     // Print a message to the LCD.
     lcd.setCursor(0,0);
     lcd.print(F("Temperature A1!"));
     lcd.setCursor(0,2);
     lcd.print(F("Distance A2!"));
     }
     lcd.setCursor(0,1);
     lcd.print(((float)getParameter(PARAM_TEMP1))/100);
     lcd.print(F(" C   "));
     lcd.setCursor(0,3);
     lcd.print(getParameter(PARAM_DISTANCE));
     lcd.print(F(" mm    "));
     } 
     else {
     clearWireFlag(wireFlag32, WIRE_LCD_20_4); 
     }
     }*/
#endif

}
