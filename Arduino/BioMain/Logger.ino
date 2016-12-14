/*****************************************************************************************
 This thread takes care of the logs and manage the time and its synchronisation 
 The thread write the logs at a definite fixed interval of time in the SST25VF064 chip 
 The time synchronization works through the NTP protocol and our server
******************************************************************************************/
#include <SST.h>

//Types of logs
#define ENTRY_SIZE_LINEAR_LOGS     64
#define NB_PARAMETERS_LINEAR_LOGS  26
#define SIZE_TIMESTAMPS            4
#define SIZE_COUNTER_ENTRY         4

// Definition of the log sectors in the flash for the logs
#if defined(SST64) || defined(SST64_OLD) //64Mbits
  #define ADDRESS_MAX   0x800000 // http://www.sst.com/dotAsset/40498.pdf&usd=2&usg=ALhdy294tEkn4s_aKwurdSetYTt_vmXQhw
#elif defined(SST32) || defined(SST32_OLD) //32Mbits
  #define ADDRESS_MAX   0X400000
#endif

#define ADDRESS_BEG   0x000000
#define ADDRESS_LAST  (ADDRESS_MAX - ENTRY_SIZE_LINEAR_LOGS)
#define SECTOR_SIZE       4096
#define NB_ENTRIES_PER_SECTOR    (SECTOR_SIZE  / ENTRY_SIZE_LINEAR_LOGS)
#define ADDRESS_SIZE  (ADDRESS_MAX  - ADDRESS_BEG)
// The number of entires by types of logs (seconds, minutes, hours, commands/events)
#define MAX_NB_ENTRIES    (ADDRESS_SIZE  / ENTRY_SIZE_LINEAR_LOGS)


#if defined(SST64) || defined(SST64_OLD) || defined(SST32) || defined(SST32_OLD)

SEMAPHORE_DECL(lockFlashAccess, 1);
SST sst=SST('B',6); //D10 is B6

static uint32_t nextEntryID = 0;
boolean logActive=false;
boolean busy_flag=false;

/*********************************************************************************** 
 Save logs in the Flash memory.
 event_number: If there is a command, then this parameter should be set with the
 corresponding command/event number. Should be found in the define list of
 commands/errors
************************************************************************************/
void writeLog() {
  writeLog(0,0);
}

void writeLog(uint16_t event_number, uint16_t parameter_value) {
  /********************************
             Safeguards
  ********************************/
  #if ! defined ( THR_LINEAR_LOGS ) 
  return;
  #endif
  if (!logActive) return;
  /*****************************
            Slave Select
  ******************************/
  nilSemWait(&lockFlashAccess);
  
  
/************************************************************************************
    Test if it is the begining of one sector, erase the sector of 4096 bytes if needed  delay(2);
  ************************************************************************************/
  if((!(nextEntryID % NB_ENTRIES_PER_SECTOR))) {
    #ifdef DEBUG_LOGS
    Serial.print(F("ERASE sctr: "));
    Serial.println(findSectorOfN());
    #endif
    sst.flashSectorErase(findSectorOfN());
  }
  /*****************************
          Writing Sequence
  ******************************/
  uint16_t param = 0;
  uint32_t timenow = now();
  uint32_t startAddress = findAddressOfEntryN(nextEntryID);
  
  #if defined(SST64) || defined(SST32) 
    sst.flashWriteInit(startAddress); // Initialize with the right address 
    sst.flashWriteNextInt32(nextEntryID);      //4 bytes of the entry number
    sst.flashWriteNextInt32(timenow);            //4 bytes of the timestamp in the memory using a mask
    for(byte i = 0; i < NB_PARAMETERS_LINEAR_LOGS; i++) {
      param = getParameter(i);
      sst.flashWriteNextInt16(param);          //2 bytes per parameter
    }
    sst.flashWriteNextInt16(event_number);    //event
    sst.flashWriteNextInt16(parameter_value); //parameter value */
    sst.flashWriteFinish();                   // finish the writing process
  
  #elif defined(SST64_OLD) || defined(SST32_OLD)
     sst.flashWriteInit(startAddress); // Initialize with the right address 
    //entry #
    oldFlashWriteNextInt32(startAddress,nextEntryID);
    startAddress+=4;
    //epoch
    oldFlashWriteNextInt32(startAddress,timenow);
    startAddress+=4;
    for(byte i = 0; i < NB_PARAMETERS_LINEAR_LOGS; i++) {
      param = getParameter(i);
      oldFlashWriteNextInt16(startAddress,param); //2 bytes per parameter
      startAddress+=2;      
    }
      oldFlashWriteNextInt16(startAddress, event_number); //event
      startAddress+=2;
      oldFlashWriteNextInt16(startAddress, parameter_value); //parameter value     
  #endif
  
  
  /*****************************
          Check Sequence
  ******************************/
  sst.flashReadInit(findAddressOfEntryN(nextEntryID));
  long writtenID=sst.flashReadNextInt32();
  #ifdef DEBUG_LOGS  
  Serial.println(F("nextEntryID "));
  Serial.println(nextEntryID);
  Serial.println(F("writtenID "));
  Serial.println(writtenID);
  #endif
  sst.flashReadFinish();
  if (writtenID==nextEntryID) {
    //Update the value of the next event log position in the memory
    nextEntryID++;
    #ifdef DEBUG_LOGS
    Serial.print(F("OK"));
    #endif

  }
  
  #ifdef DEBUG_LOGS
  else{
    Serial.print(F("Fail"));
  }
  #endif
  /*****************************
         Out and Deselect
  ******************************/
  nilThdSleepMilliseconds(5);
  nilSemSignal(&lockFlashAccess);
}

/******************************************************************************************
 Read the corresponding logs in the flash memory of the entry number (ID).
 result: Array of uint8_t where the logs are stored. It should be a 32 bytes array
 for the 3 RRD logs and 12 bytes for the commands/events logs.  
    }
  }
  formatFlash(output);
  nextEntryID=0;
}

#ifdef LOG_INTERVAL
 *entryN: Log ID that will correspond to the logs address to be read and stored in result
 return:  Error flag: 0: no error occured
 EVENT_ERROR_NOT_FOUND_ENTRY_N: The log ID (entryN) was not found in the flash memory
 *****************************************************************************************/
uint32_t printLogN(Print* output, uint32_t entryN) {
   
  nilSemWait(&lockFlashAccess);
  // Are we asking for a log entry that is not on the card anymore ? Then we just start with the first that is on the card
  // And we skip a sector ...
  if ((nextEntryID > MAX_NB_ENTRIES) && (entryN < (nextEntryID - MAX_NB_ENTRIES + NB_ENTRIES_PER_SECTOR))) {
    entryN=nextEntryID - MAX_NB_ENTRIES + NB_ENTRIES_PER_SECTOR;
  }
  sst.flashReadInit(findAddressOfEntryN(entryN));
  #ifdef DEBUG_LOGS
  Serial.print(F("entryN: "));
  Serial.println(entryN);
  #endif
  byte checkDigit=0;
  for(byte i = 0; i < ENTRY_SIZE_LINEAR_LOGS; i++) {
    byte oneByte=sst.flashReadNextInt8();
    checkDigit^=toHex(output, oneByte);
  }
  checkDigit^=toHex(output, (int)getQualifier());
  toHex(output, checkDigit);
  output->println("");
  sst.flashReadFinish();
  nilSemSignal(&lockFlashAccess);
  return entryN;
}


void Last_Log_To_SPI_buff(byte* buff) {
   
  nilSemWait(&lockFlashAccess);
  sst.flashReadInit(findAddressOfEntryN(nextEntryID-1));
  for(byte i = 0; i < ENTRY_SIZE_LINEAR_LOGS; i++) {
    byte oneByte=sst.flashReadNextInt8();
    buff[i]=oneByte;
  }
  sst.flashReadFinish();
  nilSemSignal(&lockFlashAccess);
}


uint8_t loadLastEntryToParameters() {
  nilSemWait(&lockFlashAccess);
  uint32_t addressOfEntryN = findAddressOfEntryN(nextEntryID-1);
  sst.flashReadInit(addressOfEntryN+8); // we skip entryID and epoch
  for(byte i = 0; i < NB_PARAMETERS_LINEAR_LOGS; i++) {
    setParameter(i,sst.flashReadNextInt16());
  }
  sst.flashReadFinish(); 
  nilSemSignal(&lockFlashAccess);
}


/*************************************************************************************
 The flash memory is implemented with sectors of a defined size.
 The function returns the sector number where the log corresponding to the ID (entryNb) 
 is stored in the flash memory
 entryNb:         The log ID
 return:          The sector number
**************************************************************************************/
uint16_t findSectorOfN( ) {
  uint16_t sectorNb = 0;
  uint32_t address = findAddressOfEntryN(nextEntryID);
  sectorNb = address / SECTOR_SIZE;
  return sectorNb;
}

/******************************************************************
  implement write support for obsolete memories for inin16 & int32
*******************************************************************/
//check if the Write Finish command is necessary each time
#if defined(SST64_OLD) || defined(SST32_OLD)
void oldFlashWriteNextInt32(uint32_t addr, uint32_t data){  
  sst.flashWriteInit(addr);
  sst.flashWriteNextInt8((byte)((data >> 24) & 0xFF));
  sst.flashWriteFinish();  
  sst.flashWriteInit(addr+1);
  sst.flashWriteNextInt8((byte)((data >> 16) & 0xFF));
  sst.flashWriteFinish(); 
  sst.flashWriteInit(addr+2);
  sst.flashWriteNextInt8((byte)((data >> 8) & 0xFF));
  sst.flashWriteFinish(); 
  sst.flashWriteInit(addr+3);
  sst.flashWriteNextInt8((byte)(data & 0xFF));
  sst.flashWriteFinish(); 
}

void oldFlashWriteNextInt16(uint32_t addr, uint16_t data){  
  sst.flashWriteInit(addr);
  sst.flashWriteNextInt8((byte)((data >> 8) & 0xFF));
  sst.flashWriteFinish();  
  sst.flashWriteInit(addr+1);
  sst.flashWriteNextInt8((byte)(data & 0xFF));
  sst.flashWriteFinish(); 
}
   
#endif

/******************************************************************************
 Returns the address corresponding to one log ID nilThdSleepMilliseconds(5); nilThdSleepMilliseconds(5);
 entryNb:     Log ID 
 return:      Address of the first byte where the corresponding log is located
*******************************************************************************/
uint32_t findAddressOfEntryN(uint32_t entryN)
{
  uint32_t address = ((entryN % MAX_NB_ENTRIES) * ENTRY_SIZE_LINEAR_LOGS) % ADDRESS_SIZE + ADDRESS_BEG;
  return address;
}

/*****************************************************************************
 Returns the last log ID stored in the memory 
 return: Last log ID stored in the memory corresponding to a log type
******************************************************************************/
void recoverLastEntryN() 
{
  uint32_t ID_temp = 0;
  uint32_t Time_temp = 0;
  uint32_t addressEntryN = ADDRESS_BEG;
  boolean found = false;

  #ifdef DEBUG_LOGS
  Serial.print(F("1st addr: "));
  Serial.println(ADDRESS_BEG);
  Serial.print(F("Max addr: "));
  Serial.println(ADDRESS_LAST);  
  #endif

  while(addressEntryN<ADDRESS_LAST) 
  {
    sst.flashReadInit(addressEntryN);
    ID_temp = sst.flashReadNextInt32();
    Time_temp = sst.flashReadNextInt32(); 
    sst.flashReadFinish();          
    
    #ifdef DEBUG_LOGS    
    Serial.print(F("ID_tmp: "));
    Serial.println(ID_temp);
    Serial.print(F("nextEntryID: "));
    Serial.println(nextEntryID);
    #endif
    
    // Test if first memory slot contains any information
    if((ID_temp == 0xFFFFFFFF) || (ID_temp < nextEntryID))
    {
      break;
    }
    addressEntryN += ENTRY_SIZE_LINEAR_LOGS;
    nextEntryID = ID_temp+1; // this will be the correct value in case of break
    setTime(Time_temp);

    // we implement a quick advance
    if (addressEntryN<(ADDRESS_LAST-128*ENTRY_SIZE_LINEAR_LOGS)) {
      sst.flashReadInit(addressEntryN+(128*ENTRY_SIZE_LINEAR_LOGS));
      ID_temp = sst.flashReadNextInt32();
      sst.flashReadFinish(); 
      if (ID_temp >= nextEntryID && ID_temp != 0xFFFFFFFF) {
        addressEntryN+=127*ENTRY_SIZE_LINEAR_LOGS;
      }
    }

    #ifdef DEBUG_LOGS
    Serial.print(F("Current nextEntryID:")); 
    Serial.println(nextEntryID);
    #endif
  }
  #ifdef DEBUG_LOGS
  Serial.print(F("Final nextEntryID:")); 
  Serial.println(nextEntryID);
  #endif
  logActive=true;
  }

/*****************************
 Memory related functions
 *****************************/
//Setup the memory for future use
//Need to be used only onced at startup
void setupMemory(){
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  sst.init();
}

void printLastLog(Print* output) {
  printLogN(output, nextEntryID-1);
}

void formatFlash(Print* output) {
  busy_flag=true;
  setupMemory();
  #ifdef DEBUG_LOGS  
  output->println(F("Format flash"));
  output->print(F("Sctr size:"));
  output->println(SECTOR_SIZE);
  output->print(F("Nb sctrs:"));
  output->println(ADDRESS_MAX/SECTOR_SIZE);
  #endif
  wdt_disable();
 
  for (int i=0; i<ADDRESS_MAX/SECTOR_SIZE; i++) {
    sst.flashSectorErase(i);
    if (i%16==0)
      output->print(F("."));
    if (i%1024==1023)
      output->println(F("")); 
    nilThdSleepMilliseconds(10);
  } 
  wdt_enable(WDTO_8S);
  wdt_reset();
  output->println(F("OK"));
  setTime(0);
  nextEntryID=0;
  busy_flag=false;
}

//need revision !!!
void validateFlash(Print* output) {
  logActive=false;
  formatFlash(output);
  output->println(F("Write / read / validate"));
  for (int i=0; i<ADDRESS_MAX/SECTOR_SIZE; i++) {
    for (byte j=0; j<SECTOR_SIZE/64; j++) {
      long address=(long)i*SECTOR_SIZE+(long)j*64;
      sst.flashWriteInit(address);
      byte result=0;
      for (byte k=0; k<64; k++) {
        result^=(k+13);
        sst.flashWriteNextInt8(k+13);
      }
      sst.flashWriteFinish();
      sst.flashReadInit(address);
      for (byte k=0; k<64; k++) {
        result^=sst.flashReadNextInt8();
      }
      sst.flashReadFinish();
      if (result==0) {
        if (j==0 && i%16==0) {
          output->print(".");
        }
        if (j==0 && i%1024==1023) {
          output->println(""); 
        }
      } 
      else {
        output->println(address);
      }
    }
  }
  formatFlash(output);
  nextEntryID=0;
}

#ifdef LOG_INTERVAL
#ifdef DEBUG_LOGS
NIL_WORKING_AREA(waThreadLogger, 120);
#else
NIL_WORKING_AREA(waThreadLogger, 64);
#endif
NIL_THREAD(ThreadLogger, arg) {
  nilThdSleepMilliseconds(5000);
  writeLog(EVENT_ARDUINO_BOOT,0);
  while(TRUE) {
    //avoids logging during the second x+1, ensure x+LOG_INTERVAL
    //because epoch is only precise to the second so the logging
    //is evenly spaced
    nilThdSleepMilliseconds(LOG_INTERVAL*1000-millis()%1000+100); 
    if(!busy_flag) writeLog(); 
  }
}

#endif
#endif



















