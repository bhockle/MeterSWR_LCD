/*
 i2cscanner- I2C Bus scanner - based on the work of Tod E. Kurk, http://todbot.com/blog
 Copyright (c) 2011 Omar Francisco
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 IN THE SOFTWARE.

*/
#include <wprogram.h>

#ifndef I2CSCANBUS_H
#define I2CSCANBUS_H    

//! This namespace contains general purpose utilities
namespace Utilities
{
    //! This class is intended to be used for the purpose of debugging.  It scans the I2C bus and generates a report of the addresses found.
    /*!
      This class requires the initialization of the wire and serial interface prior to its usage.
    */
    class I2CScanBus
    {
        public:

        //! Numeric system used to display the address scanned on the bus.
        enum NumberSystem
        {
            NUM_DEC_HEX = 0, /*!< Display address using both dec and hex systems. */
            NUM_BIN = 2,    /*!< Display address using binary system. */
            NUM_OCT = 8,    /*!< Display address using octal system. */
            NUM_DEC = 10,   /*!< Display address using decimal system. */
            NUM_HEX = 16    /*!< Display address using hexadecimal system. */

        };

        //! Number of columns used to display output of bus scan.  
        enum DisplayFormat
        {
            ONE_COLUMN   = 1,   /*!< Display output in 1 column. */
            TWO_COLUMN   = 2,   /*!< Display output in 2 column. */
            THREE_COLUMN = 3,   /*!< Display output in 3 column. */
            FOUR_COLUMN  = 4,   /*!< Display output in 4 column. */
            FIVE_COLUMN  = 5,   /*!< Display output in 5 column. */
            SIX_COLUMN   = 6    /*!< Display output in 6 column. */
        };

        //! Scan the I2C bus from address 1-127 and display address in decimal and hex format using four columns.
        static void scanBus();

        //! Scan the I2C bus from start address to end address.  Display address in decimal and hex format using four columns.
        /*!
            This method requires the initialization of the wire and serial interface prior to its usage. 
            \param startAddress The first address to scan.
            \param endAddress The last address to scan.
        */
        static void scanBus(byte startAddress, byte endAddress);

        //! Scan the I2C bus.  All paremeters can be specified.
        /*!
            This method requires the initialization of the wire and serial interface prior to its usage. 
            \param startAddress The first address to scan.
            \param endAddress The last address to scan.
            \param numSystem Numeric system to display bus address.
            \param numberOfColumns Number of columns to display report.
        */
        static void scanBus(byte startAddress, byte endAddress, enum NumberSystem numSystem, enum DisplayFormat numberOfColumns);

        //! Check if an address is on the I2C bus. Returns true if the address if found, false otherwise.
        /*!
            This method requires the initialization of the wire and serial interface prior to its usage. 
            \param address The address to search in the I2C bus.
        */
        static bool isAddressOnBus( byte address );
    };
}

#endif