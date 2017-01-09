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
		SST(int pin);
		void init();
		//We only need 24 bits here
		void flashReadInit(uint32_t);

		uint8_t flashReadNextInt8();
		uint16_t flashReadNextInt16();
		uint32_t flashReadNextInt32();

		void flashReadFinish();
		void flashWriteInit(uint32_t);

		void flashWriteNextInt8(uint8_t);
		void flashWriteNextInt16(uint16_t);
		void flashWriteNextInt32(uint32_t);
		void flashWriteFinish();
		void flashSectorErase(uint16_t);
		void flashTotalErase();
		//this function could be used to get the position in the memory after a reboot
		void printNonEmptySector();
		void printFlashID(Print*);
	private: 
		int _ssPin;
		inline void volatile nop(void);
		void flashEnable();
		void flashDisable();
		void flashInit();
		
		void flashWaitUntilDone();
		void flashSetAddress(uint32_t);
};

#endif
