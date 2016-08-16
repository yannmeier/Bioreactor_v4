/*
  EXROM_write.cpp - Extended EEPROM library
  Written/extended by Tom Bloor aka. TBSliver.
  
  Based on:
  
  EEPROM library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <avr/eeprom.h>
#include "Arduino.h"
#include "EXROM.h"

//          Write Functions

void EXROMClass::write(int writePointer, byte writeDataStore)                   //write a byte to EEPROM
{
    eeprom_write_byte((unsigned char *) writePointer, writeDataStore);
}

void EXROMClass::write(int writePointer, byte writeDataStore[], int writeSize)  //write a byte array to EEPROM
{
  for (int i = 0; i < writeSize; i++)
  {
    int j = writePointer + i;
    write(j, writeDataStore[i]);
  }
}

void EXROMClass::write(int writePointer, char writeDataStore)                   //write a char to EEPROM
{
  write(writePointer, byte(writeDataStore));
}

void EXROMClass::write(int writePointer, char writeDataStore[], int writeSize)  //write a string to EEPROM
{
  for (int i = 0; i < writeSize; i++)
  {
    int j = writePointer + i;
    write(j, byte(writeDataStore[i]));
  }
}

void EXROMClass::write(int writePointer, int writeDataStore)                    //write an int to EEPROM
{
  union IntToArray {
    int i;
    byte b[2];
  } ita;
  ita.i = writeDataStore;
  write(writePointer, ita.b, 2);
}

void EXROMClass::write(int writePointer, int writeDataStore[], int writeSize)   //write an int array to EEPROM
{
  for(int i=0; i < writeSize/2; i++)
  {
    int j = writePointer + i*2;
    write(j, writeDataStore[i]);
  }
}

void EXROMClass::write(int writePointer, float writeDataStore)                  //write a float to EEPROM
{
  union FloatToArray {
    float f;
    byte b[4];
  } fta;
  fta.f=writeDataStore;
  write(writePointer, fta.b, 4);
}

void EXROMClass::write(int writePointer, float writeDataStore[], int writeSize) //write a float array to EEPROM
{
  for(int i=0; i < writeSize/4; i++)
  {
    int j = writePointer + i*4;
    write(j, writeDataStore[i]);
  }
}

void EXROMClass::write(int writePointer, long writeDataStore)                  //write a long to EEPROM
{
  union LongToArray {
    long l;
    byte b[4];
  } lta;
  lta.l=writeDataStore;
  write(writePointer, lta.b, 4);
}

void EXROMClass::write(int writePointer, long writeDataStore[], int writeSize) //write a long array to EEPROM
{
  for(int i=0; i < writeSize/4; i++)
  {
    int j = writePointer + i*4;
    write(j, writeDataStore[i]);
  }
}

EXROMClass EXROM;