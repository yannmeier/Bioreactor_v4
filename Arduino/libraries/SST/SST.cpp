#include "Arduino.h"
#include "SST.h"
#include "SPI.h"

/**********************************************************************************
 * This library is an updated library from the SST library compatible with    	  *
 * he SST25VF chip. It has been designed to be compatible with an SST26VF     	  *
 * chip. A new attribute has been added which expresses whether the chip is   	  *
 * SST25 or SST26. Implementation is then conditionnal depending on its type. 	  *
 * 									      	  *
 * HOW TO USE								      	  *
 * ==========								      	  *
 * 										  *
 * Object SST has to be initialized with port name and pin number.		  *
 * 										  *
 * Before unsing (after construction), the init function has to be run, in order  *
 * to write the status and configuration registers.				  *
 * 										  *
 * When launching READ sequence, first use the flashReadInit function, then use	  *
 * the corresponding method (depending on the desired type). When done reading,	  *
 * enter flashReadFinish.							  *
 * 										  *
 * When launching WRITE sequence, first use the flashWriteInit function, then use *
 * the corresponding method (depending on the desired type). When done reading,	  *
 * enter flashWriteFinish.							  *
 * 										  *
 * Two methods are available for erasing. The Sector Erase and the total erase.   *
 * printFlashID method allows to print the flash ID on output and displays	  *
 * 	CONSTRUCTOR ID | DEVICE TYPE (25 or 26) | DEVICE ID			  *
 **********************************************************************************/

//PUBLIC METHODS----------------------------------------------------

// Still need to verify if retrocompatibility works

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
	
	/*
	 * Initialization of flashVersion attribute
	 * This should be verified as it may not work
	 */
	 
	 /* 
	  * As default, we will consider SST26VF as the default chip and
	  * these settings will be used.
	  * flashVersion is defined in init function 
	  */
	 
	 flashVersion = 0;
	 
}

void SST::init()
{
  /********************************************
   * Initialisation of flashVersion attribute *
   * This can't be done in constructor as SPI *
   *   protocol is not defined at that time.  *
   ********************************************/
      
      flashEnable();
      
      //read ID command
      (void) SPI.transfer(0x9F);
      int temp;
      // Constructor ID
      temp = SPI.transfer(0);
      // Device Type (25H if SST25 or 26H if SST26)
      flashVersion = SPI.transfer(0);
      // Device ID
      temp = SPI.transfer(0);
      //nop();
      
      flashDisable();

  /******************************
   * Initialisation of SST chip *
   * Writing of both status and *
   *   configuration register.  *
   ******************************/

  flashEnable();
  *memPort &=~(_BV(_ssPin));
  
  switch(flashVersion)
  {
    case 0x25:
      SPI.transfer(0x50); // EWSR : Enable Write Status Register, must be issued prior to WRSR
      break;
    // default is SST26
    default:
      SPI.transfer(0x06); // WREN : Write-enable instruction, must be issued prior to WRSR
      break;
    }
      
  
  *memPort |= _BV(_ssPin);
  delay(50);	// TO BE VERIFIED
  *memPort &=~(_BV(_ssPin));
  
  SPI.transfer(0x01); //write the status register instruction
  
  // If SST25VF: Only consider the Status register command, bytes sent must be xx0000xx to remove all block protection
  // If SST26VF: Status register is read only, bytes sent don't matter. Status register must be followed by configuration register
  // Bytes for configuration register must be x1xxxxx0
  
  // Status register
  SPI.transfer(0x00);
  // Configuration register
  //~ if(flashVersion != 0x25) //$$$$$$
      SPI.transfer(0x50); // ONLY FOR SST26VF
  *memPort |= _BV(_ssPin);
  delay(50);	// TO BE VERIFIED
  flashDisable();
}

//check OF NON EMPTY sectors
void SST::printNonEmptySector(Print* output){
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
		  output->print("Sector: ");
		  if ((x+1) < 10) output->print("00"); else if ((x+1) < 100) output->print("0");
		  output->println(x+1, DEC);
		}
		flashReadFinish();
	}
}

void SST::printFlashID(Print* output)
{
  uint8_t id, mtype, dev;
  flashEnable();
  flashWaitUntilDone();
  *memPort &=~(_BV(_ssPin));
  // Read ID command
  (void) SPI.transfer(0x9F);
  // Constructor ID
  id = SPI.transfer(0);
  // Device Type (25 if SST25 or 26 if SST26)
  mtype = SPI.transfer(0);
  // Device ID
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
	// In SST26, WREN has already been issued at initialisation
	if(flashVersion == 0x25)
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

void SST::flashSectorErase(uint16_t sectorAddress)
{
  flashEnable();
  *memPort &=~(_BV(_ssPin));
  // In SST26, WREN has already been issued at initialisation
 if(flashVersion == 0x25)
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
  // In SST26, WREN has already been issued at initialisation
  // Must verify if needed again but it should work like that
 // if(flashVersion == 0x25)	//$$$$
      SPI.transfer(0x06);//write enable instruction
  *memPort |= _BV(_ssPin);
  nop();
  *memPort &=~(_BV(_ssPin));
  if(flashVersion == 0x25)
      (void) SPI.transfer(0x60); // Erase Chip //
  else 	// 0x60 operation not supported by SST26
      (void) SPI.transfer(0x7C); // Erase Chip //
  *memPort |= _BV(_ssPin);
  flashWaitUntilDone();
  flashDisable();
}


// PRIVATE METHODS -------------------------------------------------
	

//magic function
// No operation : Processor will ignore the instruction. Increments counter
inline void volatile SST::nop(void) { asm __volatile__ ("nop"); }

void SST::flashEnable()    { SPI.setBitOrder(MSBFIRST); nop(); }
void SST::flashDisable()   { SPI.setBitOrder(LSBFIRST); nop(); }	// Do we need the setBitOrder?

void SST::flashWaitUntilDone()
{
  uint8_t data = 0;
  while (1)
  {
    *memPort &= ~(_BV(_ssPin));
    (void) SPI.transfer(0x05);
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





