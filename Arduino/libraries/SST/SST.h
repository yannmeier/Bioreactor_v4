/*
	flashSST.h - Library for controlling the chip SST25VF064
	Created by Gael Grosch, July 18, 2013
	With the help of the www.beat707.com code
	Released in the public domain

	SST chip has a size of 64Mb
	Reading and writing is byte per byte
	Erasing is only possible per sectors of 4096 Byte. There are 2048 such sectors.

	The addressing works with 3 bytes. The addresses are contained between 0h000000 and 0h7FFFFF
*/

#ifndef SST_h
#define SST_h

#include "Arduino.h"

class SST
{
	public:
	
	//PUBLIC METHODS----------------------------------------------------
	
		SST(char port, int pin);
		void init();
		void printNonEmptySector();	//this function could be used to get the position in the memory after a reboot
		void printFlashID(Print*);
			
		/********
		 * READ *
		 ********/
		
		void flashReadInit(uint32_t);
	
		uint8_t flashReadNextInt8();
		uint16_t flashReadNextInt16();
		uint32_t flashReadNextInt32();
		
		void flashReadFinish();
	
		/*********
		 * WRITE *
		 *********/
		 
		void flashWriteInit(uint32_t);

		void flashWriteNextInt8(uint8_t);
		void flashWriteNextInt16(uint16_t);
		void flashWriteNextInt32(uint32_t);
		
		void flashWriteFinish();
		
		/*********
		 * ERASE *
		 *********/
		
		void flashSectorErase(uint16_t);
		void flashTotalErase();
		
	private:
	
	// CLASS ATTRIBUTES ------------------------------------------------
	
		volatile uint8_t *memPort;
		int _ssPin;
		int flashVersion;
	
	// PRIVATE METHODS -------------------------------------------------
	
		// "No operation" function: Specifies to the controller to wait for 1 clock round
		inline void volatile nop(void);
		
		void flashEnable();
		void flashDisable();
		void flashInit();

		void flashWaitUntilDone();
		void flashSetAddress(uint32_t);
};

#endif
