#include "i2cscanner.h"
#include <wprogram.h>

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
  #include "utility/twi.h"
}

using Utilities::I2CScanBus;

const byte FIRST_I2C_ADDRESS = 1;
const byte LAST_I2C_ADDRESS = 127;
const char* const REPORT_HEADER = "\nI2C Bus Scan Report";

// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte startAddress, byte endAddress,byte numSystem, byte displayFormat, 
                void(*callback)(byte address, byte result, byte numSystem, byte displayFormat) ) 
{
  byte rc;
  byte dummyData = 0; 
  for( byte addr = startAddress; addr <= endAddress; addr++ ) {
    rc = twi_writeTo(addr, &dummyData, 0, 1);
    callback( addr, rc, numSystem, displayFormat );
  }
}

// Called when address is found in scanI2CBus()
// Feel free to change this as needed
// (like adding I2C comm code to figure out what kind of I2C device is there)
void scanFunc( byte addr, byte result, byte numSystem, byte displayFormat ) {
  Serial.print("addr: ");
  if ( numSystem == I2CScanBus::NUM_DEC_HEX )
  {
    Serial.print(addr,DEC);
    Serial.print("(");
    Serial.print(addr,HEX);
    Serial.print(")");
  }
  else
      Serial.print(addr, numSystem);
  Serial.print( (result==0) ? " (found!)":"       ");
  Serial.print( (addr % displayFormat) ? "\t":"\n");
}


void I2CScanBus::scanBus()
{
    scanBus( FIRST_I2C_ADDRESS, LAST_I2C_ADDRESS);
}

void I2CScanBus::scanBus(byte  startAddress, byte endAddress)
{
    scanBus( startAddress, endAddress, NUM_DEC_HEX, FOUR_COLUMN);
}

void I2CScanBus::scanBus(byte startAddress, byte endAddress, enum NumberSystem numSystem, enum DisplayFormat displayFormat)
{
    Serial.print(REPORT_HEADER);
    Serial.print(" (");
    Serial.print( numSystem == 0 ? "DEC+HEX" : numSystem == DEC ? "DEC" : numSystem == BIN ? "BIN" : numSystem == OCT ? "OCT" : numSystem == HEX ? "HEX" : "UNKNOWN");
    Serial.println(") ");
    scanI2CBus ( startAddress, endAddress, (byte)numSystem, (byte)displayFormat, scanFunc);
}


bool I2CScanBus::isAddressOnBus( byte addr )
{
    byte rc;
    byte dummyData = 0; 
    rc = twi_writeTo(addr, &dummyData, 0, 1);
    return rc==0 ? true : false;
}


