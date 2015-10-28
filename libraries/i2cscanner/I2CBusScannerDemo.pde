#include <Wire.h>
#include <i2cscanner.h>
using Utilities::I2CScanBus;
void setup()
{
    /* Initialize libraries */ 
    Wire.begin();
    Serial.begin(19200);    
    
    /* Scan I2C bus from address 1 - 127 */
    I2CScanBus::scanBus();
    
    /* Check if there is a device at address 103 */
    Serial.print("\nSearching for address 103...");
    if ( I2CScanBus::isAddressOnBus(103) )
        Serial.println("Device Found");
    else
        Serial.println("Device NOT Found"); 
     
    /* Scan I2C bus from address 10-20 */   
    I2CScanBus::scanBus(10,20);
    
    /* Scan I2C bus from address 1 - 127, display addresses in HEX using two columns */
    I2CScanBus::scanBus(1,127, I2CScanBus::NUM_HEX, I2CScanBus::TWO_COLUMN);  
}

void loop()
{
  
}
