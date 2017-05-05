#include "Arduino.h"
#include "SST.h"
#include "SPI.h"


//PUBLIC METHODS----------------------------------------------------

SST::SST(char port, int pin)
{
	pinMode(pin, OUTPUT);
	switch (port) {
		#ifdef PORTA
		case 'A':
			memPort= &PORTA;
			break;
		#endif
		#ifdef PORTB
		case 'B':
			memPort= &PORTB;
			break;
		#endif
		#ifdef PORTC
		case 'C':
			memPort= &PORTC;
			break;
		#endif
		#ifdef PORTD
		case 'D':
			memPort= &PORTD;
			break;
		#endif
		#ifdef PORTE
		case 'E':
			memPort= &PORTE;
			break;
		#endif
		#ifdef PORTF
		case 'F':
			memPort= &PORTF;
			break;
		#endif
		default:
			memPort= NULL;
			break; //exception handling not supported
	}
	_ssPin = pin;
}

void SST::init(){
	flashInit();
	//printFlashID();
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

void SST::printFlashID(Print* output)
{
  uint8_t id, mtype, dev;
  flashEnable();
  *memPort &=~(_BV(_ssPin));
  (void) SPI.transfer(0x9F); // Read ID command
  id = SPI.transfer(0);
  mtype = SPI.transfer(0);
  dev = SPI.transfer(0);
  char buf[16] = {0};
  sprintf(buf, "%02X %02X %02X", id, mtype, dev);
  *memPort |= _BV(_ssPin);
  flashDisable();
  output->println(buf);
}

     /********
      * READ *
      ********/

void SST::flashReadInit(uint32_t addr){
	flashEnable();
	*memPort &=~(_BV(_ssPin));
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
	*memPort |= _BV(_ssPin);
	flashDisable();
}

     /*********
      * WRITE *
      *********/
      
void SST::flashWriteInit(uint32_t address){
	flashEnable();
	*memPort &=~(_BV(_ssPin));
	SPI.transfer(0x06);//write enable instruction
	*memPort |= _BV(_ssPin);
	nop();
	*memPort &=~(_BV(_ssPin));
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
	*memPort |= _BV(_ssPin);
	flashWaitUntilDone();
	flashDisable();
}

     /*********
      * ERASE *
      *********/
      
/*
Erase 4KB sectors - time needed : 18ms
	Can only erase block of size 4096 byte => 2048 sectors on the chip

	It is possible to erase larger area according to the datasheet :
	64 KByte Block-Erase of memory array SPI : 1101 1000b (D8H) 3 0 0
*/

// COMMANDS WILL PROBABLY HAVE TO BE MODIFIED
void SST::flashSectorErase(uint16_t sectorAddress)
{
  flashEnable();
  *memPort &=~(_BV(_ssPin));
  SPI.transfer(0x06);//write enable instruction
  *memPort |= _BV(_ssPin);
  nop();
  *memPort &=~(_BV(_ssPin));
  (void) SPI.transfer(0x20); // Erase 4KB Sector //
  flashSetAddress(4096UL*long(sectorAddress));
  *memPort |= _BV(_ssPin);
  flashWaitUntilDone();
  flashDisable();
}

void SST::flashTotalErase()
{
  flashEnable();
  *memPort &=~(_BV(_ssPin));
  SPI.transfer(0x06);//write enable instruction
  *memPort |= _BV(_ssPin);
  nop();
  *memPort &=~(_BV(_ssPin));
  (void) SPI.transfer(0x60); // Erase Chip //
  *memPort |= _BV(_ssPin);
  flashWaitUntilDone();
  flashDisable();
}


// PRIVATE METHODS -------------------------------------------------
	

//magic function
// No operation : Processor will ignore the instruction. Increments counter
inline void volatile SST::nop(void) { asm __volatile__ ("nop"); }

void SST::flashEnable()    { SPI.setBitOrder(MSBFIRST); nop(); }
void SST::flashDisable()   { /*SPI.setBitOrder(LSBFIRST);*/ nop(); }

void SST::flashInit()
{

  flashEnable();
  // ???
  *memPort &=~(_BV(_ssPin));
  
 // SPI.transfer(0x50); //enable write status register instruction Invalid for SST26VF
  SPI.transfer(0x06);	// WREN : Write-enable instruction, must be issued prior to WRSR
  
  // ???
  *memPort |= _BV(_ssPin);
  delay(50);
  *memPort &=~(_BV(_ssPin));
  
  SPI.transfer(0x01); //write the status register instruction
  
  // If SST25VF: Only consider the Status register command, bytes sent must be xx0000xx to remove all block protection
  // If SST25VF: Status register is read only, bytes sent don't matter. Status register must be followed by configuration register
  // Bytes for configuration register must be x1xxxxx0 <-- TO BE VERIFIED
  
  // Status register
  SPI.transfer(0x00); //value to write to register - xx0000xx will remove all block protection
  // Configuration register
  SPI.transfer(0x50); // ONLY FOR SST26VF
  *memPort |= _BV(_ssPin);
  delay(50);
  flashDisable();	// Why?
}

void SST::flashWaitUntilDone()
{
  uint8_t data = 0;
  while (1)
  {
    *memPort &= ~(_BV(_ssPin));
    (void) SPI.transfer(0x05);	// RDSR
    data = SPI.transfer(0);
    *memPort |= _BV(_ssPin);
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




