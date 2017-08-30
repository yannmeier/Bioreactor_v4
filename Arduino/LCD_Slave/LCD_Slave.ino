
/**********************************************************************************************
   LCD_SLAVE is an Arduino programm conceived for the Open-source bioreactor programm.
   This code features a programm to run on the LCD annex board to the project. It features:

   - SPI-communication with the main board in order to be able to set parameters without help
     from the computer Serial
   - A graphic menu interface allowing to view access certain commands from the bioreactor.
     The menu is architectured as such:

      MENU_SELECTOR
        |
        |- MENU_SENSOR
        |
        |- MENU_CONFIG ------|
        |               MENU_SET_VALUE
        |
        |- MENU_CALIB

   With the menus containing:

   -> MENU_SELECTOR: Home menu.
   -> MENU_SENSOR: Visualizing useful parameters. Display only. Can't be modified.
                    - Stepper RPM
                    - Current temperature
                    - Current weight
   -> MENU_CONFIG: List of settings that can be updated to set-up the bioreactor.
                    - Stepper speed
                    - Target temperature
                    - Max and min weight
                    - Sedimentation and filled times
   -> MENU_SET_VALUE: Choose new value for configurable parameter and parse and send it to
                      BioMain programm.
   -> MENU_CALIB: Weight calibration menu.
                    - Tare calibration
                    - 1kg calibration
 **********************************************************************************************/

#include <LiquidCrystal.h>
#include <SPI.h>
#include "LCD_Slave.h"

// initialize the lib with the nbr of the interface pins
LiquidCrystal lcd(LCDRS, LCDE, LCDD4, LCDD5, LCDD6, LCDD7);

//-------------------------------------------------------------------------------------------------------------------------//

/*************************
   CONSTANTS DEFINITIONS
 *************************/

// MENU POSITION LABELS
// --------------------

volatile byte encoderCurrentMenu = MENU_SELECTOR;     // Page we are currently in
volatile byte encoderPreviousMenu = MENU_SELECTOR;    // Previous page
volatile unsigned encoderSelector = 0;                // Cursor position in our menu
volatile unsigned int encoderPreviousSelection = 0;   // Cursor position in previous menu

// MENU AND VALUES REFRESH
// -----------------------

static boolean refreshMenu = true;    // Menu is only refreshed when button is pushed or in sensors menu when the wheel is turned
static boolean processIt = true;      // Check if values have been changed by either SPI interrupt or user param modification

// DEBOUNCE AND ISR VARIABLES
// --------------------------

static boolean rotating = false;  // debounce management
static boolean pushing  = true;   // debounce management
boolean A_set = false;            // ISR variable
boolean B_set = false;            // ISR variable
boolean accelerationMode = false; // When button is turned fast, variables change faster in Values setting menu
long lastEvent = millis();        // Timecode of previous rotating event: utilitary for acceleration mode

// SPI COMMUNICATION WITH MASTER
// -----------------------------

byte outBuf [OUT_BUF_SIZE];       // output SPI buffer
boolean writeToMaster = false;    // flag indicating if there is something to send
boolean isStart = false;          // flag indicating if beginning of the COM
byte buf [2 * MAX_PARAM + 1];     // buffer for input from motherboard : maxparams + XOR
volatile byte pos;                // position of incoming byte in SPI buffer
uint16_t param [MAX_PARAM] = {0}; // Table to stock parameters values, initialized to an array of 0

//-------------------------------------------------------------------------------------------------------------------------//

/***********************
   PARAMETER UTILITIES
 ***********************/

typedef struct {
  char* paramName = 0;
  int paramNameLength = 0;
  int paramNum = 0; // paramNum, gives the parameters number in the parameters table (bioMain)
} parameter;

parameter * configMenuParams [MAX_CONFIG_PARAM];  // Configurable parameters
parameter * weightMenuParams [MAX_WEIGHT_PARAM];  // Weight calib parameters

byte weightParamCalib = 0;                        // Weight parameter to calibrate

// Utilitary function: Prints parameter infos on Serial
void printParam(int pos) {
  if (pos >= 0 && pos < MAX_CONFIG_PARAM) {
    parameter * param = configMenuParams[pos];
    Serial.print(F("ParamName:"));
    Serial.println(param->paramName);
    Serial.print(F("Param#:"));
    Serial.println(param->paramNum);
  } else {
    Serial.println(F("Err: pos of config variable out of range"));
  }
}

// Add new parameter to table of parameters
void addConfigParam(int pos, char * id, int idLength, int number) {

  if (pos >= 0 && pos < MAX_CONFIG_PARAM) {
    parameter * newParam = (parameter *) malloc(idLength + 2 * sizeof(int)); // memory allocation
    newParam->paramName = id;
    newParam->paramNameLength = idLength;
    newParam->paramNum = number;
    configMenuParams[pos] = newParam;

    printParam(pos);
  } else {
    Serial.println(F("Err: pos of config variable is out of range"));
  }
}

// Add new parameter to table of parameters
void addWeightParam(int pos, char * id, int idLength, int number) {

  if (pos >= 0 && pos < MAX_WEIGHT_PARAM) {
    parameter * newParam = (parameter *) malloc(idLength + 2 * sizeof(int)); // memory allocation
    newParam->paramName = id;
    newParam->paramNameLength = idLength;
    newParam->paramNum = number;
    weightMenuParams[pos] = newParam;

    printParam(pos);
  } else {
    Serial.println(F("Err: pos of config variable is out of range"));
  }
}

//-------------------------------------------------------------------------------------------------------------------------//

/*********************
   DISPLAY UTILITIES
 *********************/

// MENU DISPLAYS
// -------------

// Main menu display function. Reroutes to each different menu
void displayMenu()
{
  switch (encoderCurrentMenu) {
    case MENU_SELECTOR:
      displayMenuSelector();
      break;
    case MENU_SENSOR:
      displayMenuSensor();
      break;
    case MENU_CONFIG:
      displayMenuConfig();
      break;
    case MENU_CALIBRATION:
      displayMenuCalibr();
      break;
    case MENU_WEIGHT_CONFIRM:
      displayWeightConfirm();
      break;
    default:
      return;
  }
  refreshMenu = false;
}

// Home menu of the bioreactor. Links to all other menus
void displayMenuSelector()
{
  lcd.begin(20, 4); // Clear Screen
  lcd.setCursor(2, 0);
  lcd.print(F("Menu Selector:"));
  lcd.setCursor(0, 1);
  lcd.print(F("1)Sensors Display"));
  lcd.setCursor(0, 2);
  lcd.print(F("2)Settings"));
  lcd.setCursor(0, 3);
  lcd.print(F("3)Scale Calibration"));
}

// Display useful values
void displayMenuSensor()
{
  lcd.begin(20, 4); // Clear Screen
  lcd.setCursor(0, 0);
  lcd.print(F("Stepper RPM:"));
  lcd.setCursor(0, 1);
  lcd.print(F("Current temp:"));
  lcd.setCursor(0, 2);
  lcd.print(F("Current weight:"));
  lcd.setCursor(16, 4);
  lcd.print(F("BACK"));
}

// Displays parameters that can be configurated. MAX_CONFIG_PARAM should be changed if and only if new config parameters are added
void displayMenuConfig()
{
  lcd.begin(20, 4); // Clear Screen
  size_t paramNb = encoderSelector % (MAX_CONFIG_PARAM + 1); // Encoder ranges to max param + 1 because of back button

  // More parameters to be displayed than lines on the screen. When scrolling up/down in menu, params move
  switch (paramNb)
  {
    // For case 0, the back button must first be displayed. Only 2 parameters are visible
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(F("0)Back"));
      for (int paramPos = 0; paramPos < 2; paramPos++)
      {
        if (paramNb + paramPos < MAX_CONFIG_PARAM)
        {
          lcd.setCursor(0, paramPos + 1);
          lcd.print(paramNb + paramPos + 1);
          lcd.print(")");
          lcd.print(configMenuParams[paramNb + paramPos]->paramName);
          lcd.print(":");
        }
      }
      break;
    // When != 0, there is no need to display back button. 3 params can be displayed at once
    default:
      for (int paramPos = 0; paramPos < 3; paramPos++)
      {
        if (paramNb + paramPos - 1 < MAX_CONFIG_PARAM) // 1) -> Param #0, 2) -> Param #1, etc..
        {
          lcd.setCursor(0, paramPos);
          lcd.print(paramNb + paramPos);
          lcd.print(")");
          lcd.print(configMenuParams[paramNb + paramPos - 1]->paramName);
          lcd.print(":");
        }
      }
      break;
  }
  lcd.setCursor(0, 4);
  lcd.print(F("..."));  // More param below, always displayed (circular menu)
}

void displayMenuCalibr()
{
  lcd.begin(20, 4); // Clear Screen

  size_t paramNb = encoderSelector % (MAX_WEIGHT_PARAM + 1); // Encoder ranges to max param + 1 because of back button

  // More parameters to be displayed than lines on the screen. When scrolling up/down in menu, params move
  switch (paramNb)
  {
    // For case 0, the back button must first be displayed. Only 2 parameters are visible
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(F("0)Back"));
      for (int paramPos = 0; paramPos < 2; paramPos++)
      {
        if (paramNb + paramPos < MAX_WEIGHT_PARAM)
        {
          lcd.setCursor(0, paramPos + 1);
          lcd.print(paramNb + paramPos + 1);
          lcd.print(")");
          lcd.print(weightMenuParams[paramNb + paramPos]->paramName);
          lcd.print(":");
        }
      }
      break;
    // When != 0, there is no need to display back button. 3 params can be displayed at once
    default:
      for (int paramPos = 0; paramPos < 3; paramPos++)
      {
        if (paramNb + paramPos - 1 < MAX_WEIGHT_PARAM) // 1) -> Param #0, 2) -> Param #1, etc..
        {
          lcd.setCursor(0, paramPos);
          lcd.print(paramNb + paramPos);
          lcd.print(")");
          lcd.print(weightMenuParams[paramNb + paramPos - 1]->paramName);
          lcd.print(":");
        }
      }
      break;
  }

  lcd.setCursor(0, 4);
  lcd.print(F("..."));  // More param below, always displayed (circular menu)
}

void displayWeightConfirm()
{
  lcd.begin(20,4);
  lcd.setCursor(1,0);
  lcd.print("Update");
  lcd. setCursor(8,0);
  lcd.print(weightMenuParams[weightParamCalib]->paramName);
  lcd.setCursor(5,2);
  lcd.print("YES");
  lcd.setCursor(12,2);
  lcd.print("NO");
}

// VALUES DISPLAY
// --------------

void valuesRefresh()
{
  switch (encoderCurrentMenu) {
    case MENU_SENSOR:
      displayValueSensor();
      break;
    case MENU_CONFIG:
      displayValueConfig();
      break;
    case MENU_CALIBRATION:
      displayValueCalib();
      break;
    case MENU_SET_VALUE:
      configSetValue();
    default:  // MENU_SELECTOR does not need to display values
      return;
  }
}

// Utilitary function to display generic parameter
void displayParamValue(int value, int space, int x, int y) {
  lcd.setCursor(x, y);
  String value_string = String(value);
  for (int i = 0; i < (space - value_string.length()); i++) { // Align values right
    lcd.print(" ");
  }
  lcd.print(value);
}

// Display values in Sensor menu
void displayValueSensor()
{
  displayParamValue(param[PARAM_TEMP_LIQ], 7, 12, 0);       //display liquid temperature
  displayParamValue(param[PARAM_STEPPER_SPEED], 6, 13, 1);  //display motor speed in RPM
  displayParamValue(param[PARAM_WEIGHT_G], 5, 14, 2);       //display weight
}

// Display values in Config menu
void displayValueConfig()
{ // Architecture is similar to menu display

  size_t paramNb = encoderSelector % (MAX_CONFIG_PARAM + 1); // Encoder ranges to max param + 1 because of back button

  // More parameters to be displayed than lines on the screen. When scrolling up/down in menu, params move
  switch (paramNb)
  {
    // For case 0, the back button must first be displayed. Only 2 parameters are visible
    case 0:
      for (int paramPos = 0; paramPos < 2; paramPos++)
      {
        if (paramNb + paramPos < MAX_CONFIG_PARAM)
          displayParamValue(param[configMenuParams[paramNb + paramPos]->paramNum], 6, 14, paramPos + 1);
      }
      break;
    // When != 0, there is no need to display back button. 3 params can be displayed at once
    default:
      for (int paramPos = 0; paramPos < 3; paramPos++)
      {
        if (paramNb + paramPos - 1 < MAX_CONFIG_PARAM) // 1) -> Param #0, 2) -> Param #1, etc..
          displayParamValue(param[configMenuParams[paramNb + paramPos - 1]->paramNum], 6, 14, paramPos);
      }
      break;
  }
}

// Display values in weight calib menu
void displayValueCalib()
{ // Architecture is similar to menu display

  size_t paramNb = encoderSelector % (MAX_WEIGHT_PARAM + 1); // Encoder ranges to max param + 1 because of back button

  // More parameters to be displayed than lines on the screen. When scrolling up/down in menu, params move
  switch (paramNb)
  {
    // For case 0, the back button must first be displayed. Only 2 parameters are visible
    case 0:
      for (int paramPos = 0; paramPos < 2; paramPos++)
      {
        if (paramNb + paramPos < MAX_CONFIG_PARAM)
          displayParamValue(param[weightMenuParams[paramNb + paramPos]->paramNum], 6, 14, paramPos + 1);
      }
      break;
    // When != 0, there is no need to display back button. 3 params can be displayed at once
    default:
      for (int paramPos = 0; paramPos < 3; paramPos++)
      {
        if (paramNb + paramPos - 1 < MAX_WEIGHT_PARAM) // 1) -> Param #0, 2) -> Param #1, etc..
          displayParamValue(param[weightMenuParams[paramNb + paramPos - 1]->paramNum], 6, 14, paramPos);
      }
      break;
  }
}

// CURSOR BLINK
// ------------

// Marks position of cursor in the screen
void cursorBlink()
{
  switch (encoderCurrentMenu)
  {
    case MENU_SELECTOR:
      lcd.setCursor(0, (encoderSelector % 3) + 1);
      break;
    case MENU_SENSOR:
      lcd.setCursor(16, 4); // Positionned on letter B of BACK
      break;
    case MENU_CONFIG:
      lcd.setCursor(0, 0); // Selected parameter is always on top of the screen
      break;               // The parameters move, not the blink
    case MENU_CALIBRATION:
      lcd.setCursor(0, 0); // Selected parameter is always on top of the screen
      break;               // The parameters move, not the blink
    case MENU_SET_VALUE:
      lcd.setCursor(19, 0);
      break;
    case MENU_WEIGHT_CONFIRM:
      lcd.setCursor(((encoderSelector%2) * 7) + 5, 2);  // Allows cursor to be positionned of Y of YES and N of NO
      break;
    default:
      lcd.setCursor(0, 0);
      break;
  }
  lcd.blink();
}

//-------------------------------------------------------------------------------------------------------------------------//

/********************************
   PARAMETER AND WEIGHT SETTING
 ********************************/

 // Change value of a parameter in Config menu and adding it to buffer to send to master
void configSetValue()
{
  byte paramNum = encoderSelector % (MAX_PARAM + 1) - 1; // Parameter to modify
  int tempVal = param[configMenuParams[paramNum]->paramNum]; // Value of parameter to set

  encoderPreviousSelection = encoderSelector;
  lcd.setCursor(19, 0); // For blink

  if (paramNum >= 0) {
    while (encoderCurrentMenu == MENU_SET_VALUE) {
      if (encoderSelector != encoderPreviousSelection) {
        if (encoderSelector > encoderPreviousSelection) {
          if (accelerationMode) tempVal += 100;
          else tempVal += 5;
        } else {
          if (accelerationMode) tempVal -= 100;
          else tempVal -= 5;
        }
        displayParamValue(tempVal, 6, 14, 0);
        encoderPreviousSelection = encoderSelector;
      }
      lcd.setCursor(19, 0); // For blink
    }
    sendParameter(configMenuParams[paramNum]->paramNum, tempVal);
    encoderSelector = paramNum + 1; // Go back to the right parameter when returning in config menu
  }
}

void updateWeightSetting()
{
  if(weightParamCalib < MAX_WEIGHT_PARAM && weightParamCalib >= 0)
  {
    if(weightParamCalib == PARAM_WEIGHT_OFFSET)
    {
      int weightToSend = param[PARAM_WEIGHT] - param[PARAM_WEIGHT_OFFSET];       // Weight factor is 1kg weight - tare
      sendParameter(configMenuParams[weightParamCalib]->paramNum, weightToSend); // Send found weight to parameters
    } else {
      sendParameter(configMenuParams[weightParamCalib]->paramNum, param[PARAM_WEIGHT]); // Stocks current weight in parameter to set
    }
  } else {
    Serial.print("WEIGHT PARAMETER OUT OF RANGE");
  }
}

//-------------------------------------------------------------------------------------------------------------------------//

/**************************************
   SPI COMMUNICATION AND DATA PARSING
 **************************************/

// ARDUINO SPI SLAVE FUNCTIONS
// ---------------------------

void slaveInit()
{
  pinMode(SS, INPUT_PULLUP);    //switch the slave select pin to input mode and turn on internal pull-up

  // turn on SPI in slave mode
  SPCR |= bit (SPE);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  // have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);

  // get ready for an interrupt
  pos = 0;   // buffer empty
  processIt = false;

  // now turn on interrupts
  SPI.attachInterrupt();
}

// DATA PARSING
// ------------

// Retrieve parameters
void bufferParse() {
  byte buffSize = buf[0];
  byte checkDigit = buffSize;
  for (int i = 0; i < buffSize; i++) { //first check that the chkDgt is ok
    checkDigit ^= buf[2 * i + 1];
    checkDigit ^= buf[2 * (i + 1)];
  }
  if (checkDigit == buf[buffSize]) { //then load parameters
    for (int i = 0; i < buffSize; i++) {
      param[i] = ((buf[2 * i + 1] << 8) & (0xFF00)) + (buf[2 * (i + 1)] & (0x00FF));
      //Serial.print(F("Changed Local Param"));
    }
  } else Serial.println(F("wrong XOR"));
}

// SPI COMMUNICATION UTILITIES
// ---------------------------

/*
   Set parameter on motherboard through SPI communication.
   int parameter : location of parameter in param list. Use defined parameters.
   int value : value to be set for the given parameter.
*/
void sendParameter(int parameter, int value) {
  outBuf[0] = 4;                                   // Start transmission with non_null byte (size of the message after)
  byte checkDigit = outBuf[0];
  outBuf[1] = (byte)parameter;                     // Number of parameter to set
  checkDigit ^= outBuf[1];
  outBuf[2] = (byte)((value >> 8) & (0x00FF));     // Value of parameter
  checkDigit ^= outBuf[2];
  outBuf[3] = (byte)(value & (0x00FF));
  checkDigit ^= outBuf[3];
  outBuf[4] = checkDigit;
  writeToMaster = true;                           // notify SPI interrupt that it can start sending bytes as soon as the transmission starts
  //Serial.println(F("Sent param"));
}

// SPI ISR
// -------

// SPI-communication interrupt with master

ISR (SPI_STC_vect)
{
  byte c = SPDR;  // Read entering byte on SPI Data Register
  byte buffSize = 0;
  
  //receive Data
  if (pos == 0) {
    isStart = true;  // start sending out bytes when the first byte is received
    buffSize = c;    // first message holds the size incoming msg
    String txt2 = String(c) + "     " + String(buffSize);
    Serial.println(txt2);
  } else if (pos <= buffSize) { // '<='  not '<'because we have a XOR at the end
    buf[pos - 1] = c;     //read until message length is reached
    if (pos == buffSize) processIt = true; //message captured
  }
  String txt = "buffSize: " + String(buffSize) + ", pos: " + String(pos) + ", processIt: " + String(processIt);
  //Serial.println(txt);

  //send Data
  if (isStart && writeToMaster) {  // send bytes if begining of COM AND if there is something
    SPDR = outBuf[pos];            // ...new to send to the master
    if (pos + 1 >= OUT_BUF_SIZE) {
      writeToMaster = false;
      isStart = false;
    }
  } else SPDR = 0;
  //increment counter
  pos++;
  //Serial.println(pos);
}


//-------------------------------------------------------------------------------------------------------------------------//

/**********************
   INTERRUPT ROUTINES
 **********************/

// INTERRUPT ON A-CHANGING STATE (Counter-clockwise)
// -----------------------------

void doEncoderA() {
  if ( rotating )
    delay(20); //debounce
  if ( digitalRead(ENCODER_CLOCKWISE) != A_set) { // debounce once more
    long current = millis();
    A_set = !A_set;

    if ( A_set && !B_set )
    {
      encoderSelector--;
      if (encoderCurrentMenu == MENU_CONFIG || encoderCurrentMenu == MENU_CALIBRATION) refreshMenu = true; // Sensor menu has to be refreshed to allow scrolling

      // When scrolling fast, acceleration mode is turned on and values in sensor menu will be incremented faster
      if (current - lastEvent < 30) accelerationMode = true;
      else accelerationMode = false;

      lastEvent = current;
    }
  }
  rotating = false;  // no more debouncing until loop() hits again
}

// INTERRUPT ON B-CHANGING STATE (Clockwise)
// -----------------------------

void doEncoderB() {
  if ( rotating )
    delay(20); //debounce
  if ( digitalRead(ENCODER_ANTI_CLOCKWISE) != B_set ) {
    long current = millis();
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set )
    {
      encoderSelector++;
      if (encoderCurrentMenu == MENU_CONFIG || encoderCurrentMenu == MENU_CALIBRATION) refreshMenu = true; // Sensor menu has to be refreshed to allow scrolling

      // When scrolling fast, acceleration mode is turned on and values in sensor menu will be incremented faster
      if (current - lastEvent < 30) accelerationMode = true;
      else accelerationMode = false;

      lastEvent = current;
    }
    rotating = false;
  }
}

// INTERRUPT ON BUTTON PRESSING
// ----------------------------

void doEncoderButton() {
  if (pushing) {
    pressMenu();      // Main pushing function
    pushing = false;  //debouncer
    refreshMenu = true; //refresh flag
  }
  else
    pushing = true;
  delay(10);
}


// DEFINED ACTION WHEN BUTTON IS PRESSED
// -------------------------------------

// Defines effect of pushing depending on current menu
void pressMenu()
{
  switch (encoderCurrentMenu) {
    case MENU_SELECTOR: // Go to selected menu
      encoderCurrentMenu = encoderSelector % 3 + 1;
      encoderSelector = 0;
      break;
    case MENU_SENSOR: // Go back to menu selection (Sensors is only display)
      encoderCurrentMenu = MENU_SELECTOR;
      encoderSelector = 0;  // Sensors menu on menu select
      break;
    case MENU_CONFIG: // ENCODER = 0 -> Back to menu selector // ENCODER != 0 -> set parameter value
      if ((encoderSelector % (MAX_CONFIG_PARAM + 1)) == 0) {
        encoderCurrentMenu = MENU_SELECTOR;
        encoderSelector = 1;  // Config menu on menu select
      } else {
        encoderCurrentMenu = MENU_SET_VALUE;
      }
      break;
    case MENU_SET_VALUE:
      encoderCurrentMenu = MENU_CONFIG;
      break;
    case MENU_CALIBRATION:
      if ((encoderSelector % (MAX_WEIGHT_PARAM + 1)) == 0) {
        encoderCurrentMenu = MENU_SELECTOR;
      }else {
        weightParamCalib = (encoderSelector-1)%MAX_WEIGHT_PARAM;  // define which weight parameter will be calibrated
        encoderCurrentMenu = MENU_WEIGHT_CONFIRM;
      }
      encoderSelector = 2;
      break;
    case MENU_WEIGHT_CONFIRM:
      if(encoderSelector%2 == 0) updateWeightSetting(); // If YES: update weight setting, else, don't
      encoderSelector = weightParamCalib + 1;
      encoderCurrentMenu = MENU_CALIBRATION; 
      break;     
    default:
      encoderCurrentMenu = MENU_SELECTOR;
      return;
  }
  refreshMenu = true; //refresh flag
}

//-------------------------------------------------------------------------------------------------------------------------//

/***********************
   SETUP AND MAIN LOOP
 ***********************/

void setup() {

  // PIN SPECIFICATIONS
  // ------------------

  // Set pins as input and pull internal resistances up
  pinMode(ENCODER_CLOCKWISE, INPUT_PULLUP);
  pinMode(ENCODER_ANTI_CLOCKWISE, INPUT_PULLUP);
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);

  // attach interrupt routines
  attachInterrupt(INT_CLOCKWISE, doEncoderA, CHANGE);
  attachInterrupt(INT_ANTI_CLOCKWISE, doEncoderB, CHANGE);
  attachInterrupt(INT_BUTTON, doEncoderButton, CHANGE);

  // SERIAL COMMUNICATION START
  // --------------------------

  Serial.begin(9600);
  slaveInit();
  lcd.begin(20, 4);
  delay(3000);  // let Serial init

  // CONFIGURABLE PARAMETERS DEFINITION
  // ----------------------------------

  addConfigParam(0, "Stepper RPM", 11, PARAM_STEPPER_SPEED);
  addConfigParam(1, "Goal temp", 12, PARAM_TEMP_TARGET);
  addConfigParam(2, "Sedim time", 10, PARAM_SEDIMENTATION_TIME);
  addConfigParam(3, "Filled time", 11, PARAM_FILLED_TIME);

  // WEIGHT CALIBRATION PARAMETERS DEFINITION
  // ----------------------------------------

  addWeightParam(0, "0kg (tare)", 10, PARAM_WEIGHT_OFFSET);
  addWeightParam(1, "1kg weight", 10, PARAM_WEIGHT_FACTOR);
  addWeightParam(2, "Min weight", 10, PARAM_WEIGHT_MIN);
  addWeightParam(3, "Max weight", 10, PARAM_WEIGHT_MAX);
}

void loop() {
  if (refreshMenu)
  {
    displayMenu();
    valuesRefresh();
  }
  if (processIt)
  {
    bufferParse();
    valuesRefresh();
    pos = 0;
    processIt = false;
  }

  cursorBlink();
  delay(20);
  rotating = true;
  //Serial.println(encoderSelector);  // Utilitary: Cursor position check
}
