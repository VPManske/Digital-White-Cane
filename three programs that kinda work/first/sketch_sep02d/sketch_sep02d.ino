/* Standard Wire.h. Givrs following error
"VL53L0X Info:
Device Name: VL53L0X ES1 or later, Type: VL53L0X, ID: 
Rev Major: 1, Minor: 15
Error expected cut 1.1 but found 1,15
VL53L0X Error: -5
tof error"

*/
#include "Adafruit_VL53L0X.h"

#define DEBUG true


int tofaddress = 0x1;

Adafruit_VL53L0X tof;

void setup() {
  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
    }

/*  
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power 
 Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
*/
 
  
 
 
    tof = Adafruit_VL53L0X();
    delay(1);
    if(!tof.begin(tofaddress, DEBUG))
    {
      Serial.println(F("tof error"));
      while(1);
    }
    delay(1);
 


  
}
void loop() {
  // put your main code here, to run repeatedly:
 


}
