#include <SST.h>
#include <SPI.h>
#include <avr/io.h>
#include <arduino.h>

#define ADDRESS_BEG   0x000000
#define ADDRESS_MAX   0x800000
#define SECTOR_SIZE       4096 // anyway the size of the sector is also hardcoded in the library !!!!

#define LINE_SIZE 64 // should be a divider of the SECTOR_SIZE

void setupMemory(SST& sst);
void printLine(long address, SST& sst);
void writeLine(long address, SST& sst);

// ======================================================================================= //

void setup()
{ 
  Serial.begin(9600);
  
  SST sst = SST('F', 4); // A3 is F4
  while(!Serial);  // Forces program to wait until Serial stream is open: Allows reading of information on setup function
                   // Maybe issue is here?
  setupMemory(sst);
  // Does not work in example. Why??
  sst.printFlashID(&Serial);

  for (long i=0; i<ADDRESS_MAX; i++) {
    if (i%SECTOR_SIZE==0) { // should erase the sector
      Serial.print("Formatting sector: ");
      Serial.println(i/SECTOR_SIZE); 
      sst.flashSectorErase(i/SECTOR_SIZE);
    }
  
   if (i%LINE_SIZE==0) printLine(i,sst);
   if (i%LINE_SIZE==0) writeLine(i,sst);
   if (i%LINE_SIZE==0) printLine(i,sst);

  }

}

// ======================================================================================= //



void loop() 
{
  
}


void printLine(long address, SST& sst) {

  Serial.print("Read Address: ");
  Serial.print(address);
  Serial.print(" : ");
  sst.flashReadInit(address);
  for (byte j=0; j<LINE_SIZE; j++) {
    byte oneByte=sst.flashReadNextInt8();
    Serial.print(oneByte, HEX);
    Serial.print(" ");
    address++;
  }
  sst.flashReadFinish();
  Serial.println("");
}

void writeLine(long address, SST& sst) {

  Serial.print("Write Address: ");
  Serial.print(address);
  Serial.print(" : ");
  sst.flashWriteInit(address);
  for (byte j=0; j<LINE_SIZE; j++) {
    sst.flashWriteNextInt8(j);
    Serial.print(j, HEX);
    Serial.print(" ");
    address++;
  }
  sst.flashReadFinish();
  Serial.println("");
}



void setupMemory(SST& sst){
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  sst.init();
}














