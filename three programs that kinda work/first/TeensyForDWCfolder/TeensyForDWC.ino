/* uses i2c_t3.h works on Teensy LC and gives no errors. 
*/
#include "Adafruit_VL53L0X.h"
#include <i2c_t3.h>

#define DEBUG true

int tofaddress = 1;

Adafruit_VL53L0X tof;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
    }
    Serial.println("step one");
 for (int i =0;i<2;i++)
  {
    tof = Adafruit_VL53L0X();
    Serial.println("step two"); 
      
    if(!tof.begin(tofaddress, DEBUG))
    {
      Serial.println(F("tof error"));
      while(1);
    }
    else Serial.println("no error");
    delay(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
 


}
