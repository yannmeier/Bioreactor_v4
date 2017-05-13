#include <SST.h>
#include <SPI.h>
#include <avr/io.h>
#include <arduino.h>

#define ADDRESS_BEG   0x000000
#define ADDRESS_MAX   0x800000
#define SECTOR_SIZE   4096 // anyway the size of the sector is also hardcoded in the library !!!!

#define LINE_SIZE 64 // should be a divider of the SECTOR_SIZE

void setupMemory(SST& sst);
void printLine(long address, SST& sst);
void writeLine(long address, SST& sst);

// ======================================================================================= //

void setup()
{
  Serial.begin(9600);

 
  while (!Serial); // Forces program to wait until Serial stream is open: Allows reading of information on setup function
  delay(200);
  SST sst = SST('F', 4); // A3 is F4
  setupMemory(sst);
  delay(100);
 
   for(int i=0; i<10;i++){sst.printConfigRegister(&Serial); delay(100);}
//  sst.flashTotalErase();
//  sst.printNonEmptySector(&Serial);
  Serial.println("the end");
  for (long i=0; i<64*2; i++) {
   if (i%LINE_SIZE==0) Serial.print("---------------------------\n");
   if (i%LINE_SIZE==0) printLine(i,sst);
   if (i%LINE_SIZE==0) writeLine(i,sst);
   if (i%LINE_SIZE==0) printLine(i,sst);
   if (i%LINE_SIZE==0) Serial.print("---------------------------\n");
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
  for (byte j = 0; j < LINE_SIZE; j++) {
    byte oneByte = sst.flashReadNextInt8();
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
  for (byte j = 0; j < LINE_SIZE; j++) {
    sst.flashWriteNextInt8(j);
    Serial.print(j, HEX);
    Serial.print(" ");
    address++;
  }
  sst.flashWriteFinish();
  Serial.println("");
}



void setupMemory(SST& sst) {
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  sst.init();
}














