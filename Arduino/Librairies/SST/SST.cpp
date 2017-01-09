#include "Arduino.h"
#include "SST.h"
#include "SPI.h"

// Create the object with the pin number of the CE pin (on the PORTB)
// This means the pin HAS TO BE ON PORT D !!!!!!!

SST::SST(int pin)
{
	pinMode(pin, OUTPUT);
	_ssPin = pin;
}

void SST::init(){
	flashInit();
//	printFlashID();	
}

//magic function
inline void volatile SST::nop(void) { asm __volatile__ ("nop"); }

void SST::flashEnable()    { SPI.setBitOrder(MSBFIRST); nop(); }
void SST::flashDisable()   { /*SPI.setBitOrder(LSBFIRST);*/ nop(); }

void SST::flashInit()
{
  flashEnable();
  PORTB &=~(_BV(_ssPin));
  SPI.transfer(0x50); //enable write status register instruction
  PORTB |= _BV(_ssPin);
  delay(50);
  PORTB &=~(_BV(_ssPin));
  SPI.transfer(0x01); //write the status register instruction
  SPI.transfer(0x00); //value to write to register - xx0000xx will remove all block protection
  PORTB |= _BV(_ssPin);
  delay(50);
  flashDisable();
}



void SST::printFlashID(Print* output)
{
  uint8_t id, mtype, dev;
  flashEnable();
  PORTB &=~(_BV(_ssPin));
  (void) SPI.transfer(0x9F); // Read ID command
  id = SPI.transfer(0);
  mtype = SPI.transfer(0);
  dev = SPI.transfer(0);
  char buf[16] = {0};
  sprintf(buf, "%02X %02X %02X", id, mtype, dev);
  PORTB |= _BV(_ssPin);
  flashDisable();
  output->println(buf);
}

void SST::flashWaitUntilDone()
{
  uint8_t data = 0;
  while (1)
  {
    PORTB &= ~(_BV(_ssPin));
    (void) SPI.transfer(0x05);
    data = SPI.transfer(0);
    PORTB |= _BV(_ssPin);
    if (!bitRead(data,0)) break;
    nop();
  }
}

void SST::flashSetAddress(uint32_t addr)
{
  (void) SPI.transfer(addr >> 16);
  (void) SPI.transfer(addr >> 8);  
  (void) SPI.transfer(addr);
}

void SST::flashReadInit(uint32_t addr){
	flashEnable();
	PORTB &=~(_BV(_ssPin));
	(void) SPI.transfer(0x03); // Read Memory - 25/33 Mhz //
	flashSetAddress(addr);
}

uint8_t SST::flashReadNextInt8() {return SPI.transfer(0); }

uint16_t SST::flashReadNextInt16() {
	uint16_t result = 0;
	// MSB
	result = SPI.transfer(0);
	result = result << 8;
	// LSB
	result = result + SPI.transfer(0);
	
	return result; 
}

uint32_t SST::flashReadNextInt32() {
	uint32_t result = 0;
	// MSB
	result = SPI.transfer(0);
	result = result << 8;
	
	result = result + SPI.transfer(0);
	result = result << 8;

	result |= result + SPI.transfer(0);
	result = result << 8;
	// LSB
	result |= result + SPI.transfer(0);
	
	return result; 
}

void SST::flashReadFinish()
{
	PORTB |= _BV(_ssPin);
	flashDisable();
}

void SST::flashWriteInit(uint32_t address){
	flashEnable();
	PORTB &=~(_BV(_ssPin));
	SPI.transfer(0x06);//write enable instruction
	PORTB |= _BV(_ssPin);
	nop();
	PORTB &=~(_BV(_ssPin));
	(void) SPI.transfer(0x02); // Write Byte //
	flashSetAddress(address);
}

//Write up to 256 byte in the memory
void SST::flashWriteNextInt8(uint8_t data)
{
	// Write Byte //
	(void) SPI.transfer(data);
}

void SST::flashWriteNextInt16(uint16_t data)
{
	flashWriteNextInt8((uint8_t)((data >> 8) & 0xFF));
	flashWriteNextInt8((uint8_t)(data & 0xFF));
}

void SST::flashWriteNextInt32(uint32_t data)
{
	flashWriteNextInt8((uint8_t)((data >> 24) & 0xFF));
	flashWriteNextInt8((uint8_t)((data >> 16) & 0xFF));
	flashWriteNextInt8((uint8_t)((data >> 8) & 0xFF));
	flashWriteNextInt8((uint8_t)(data & 0xFF));
}



void SST::flashWriteFinish(){
	PORTB |= _BV(_ssPin);
	flashWaitUntilDone();
	flashDisable();
}

/*
Erase 4KB sectors - time needed : 18ms
	Can only erase block of size 4096 byte => 2048 sectors on the chip
	
	It is possible to erase larger area according to the datasheet :
	64 KByte Block-Erase of memory array SPI : 1101 1000b (D8H) 3 0 0
*/
void SST::flashSectorErase(uint16_t sectorAddress)
{
  flashEnable();
  PORTB &=~(_BV(_ssPin));
  SPI.transfer(0x06);//write enable instruction
  PORTB |= _BV(_ssPin);
  nop();
  PORTB &=~(_BV(_ssPin));
  (void) SPI.transfer(0x20); // Erase 4KB Sector //
  flashSetAddress(4096UL*long(sectorAddress));
  PORTB |= _BV(_ssPin);
  flashWaitUntilDone();
  flashDisable();
}

void SST::flashTotalErase()
{
  flashEnable();
  PORTB &=~(_BV(_ssPin));
  SPI.transfer(0x06);//write enable instruction
  PORTB |= _BV(_ssPin);
  nop();
  PORTB &=~(_BV(_ssPin));   
  (void) SPI.transfer(0x60); // Erase Chip //
  PORTB |= _BV(_ssPin);
  flashWaitUntilDone();
  flashDisable();
}

//check OF NON EMPTY sectors
void SST::printNonEmptySector(){
	//The memory contains 2048 sectors of 4096 bytes
	for (long x = 0; x < 2048; x++)
	  {
		flashReadInit((4096UL*x));
		
		boolean sectorEmpty = true;
		for (int q=0; q<4096; q++)
		{
		  if (flashReadNextInt8() != 0xFF) { sectorEmpty = false; break; }
		}
		
		if (!sectorEmpty)
		{
		  Serial.print("Sector: ");
		  if ((x+1) < 10) Serial.print("00"); else if ((x+1) < 100) Serial.print("0");
		  Serial.println(x+1, DEC);
		}
		flashReadFinish();
	}
}  
