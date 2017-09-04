/* uses i2c_t3.h works on Teensy LC and gives no errors. 
*/
#include "Adafruit_VL53L0X_local.h"
#include "i2c_t3_local.h"

#define DEBUG true

int num_of_sensors = 1;

int xshut[5] = {0,3,5,7,9};
int gpio1[5] = {3,6,8,10,12};
int tofaddress[5] = {0x1,0x2,0x3,0x4,0x5};

Adafruit_VL53L0X tof[5];

void setup() 
{

  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) 
  {
    delay(1);
  }

  Serial.println("step one");

  for (int i = 0; i < num_of_sensors; i++)
  {
    pinMode(xshut[i], OUTPUT);
    digitalWrite(xshut[i], HIGH);
    pinMode(gpio1[i], INPUT);
  }

  Serial.println("step two"); 
  for (int i =0;i<num_of_sensors;i++)
  {
    tof[i] = Adafruit_VL53L0X();

    digitalWrite(xshut[i], LOW);
    delay(1);

    if(!tof[i].begin(tofaddress[i], DEBUG))
    {
      Serial.println(F("tof error"));
      while(1);
    }
    delay(1);
  }
  Serial.println("no error\n Setup finished\n");
}


void loop() 
{
  
  VL53L0X_RangingMeasurementData_t measure[5];

  for(int i=0;i<num_of_sensors;i++)
  {
    tof[i].rangingTest(&measure[i], DEBUG);
    if (measure[i].RangeStatus !=4) 
    {
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" Distance (mm): ");
      Serial.println(measure[i].RangeMilliMeter);
    }
    else
      Serial.println(" out of range ");
    delay(100);
  } 


}
