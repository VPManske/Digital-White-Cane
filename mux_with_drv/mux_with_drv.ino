#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
}

bool first = true;
uint8_t effect = 1;
void loop() {
  if (first == true)
{
  // put your main code here, to run repeatedly:
Wire.beginTransmission(0x70);
Wire.write(0x5);
Wire.endTransmission();


  drv.begin();
  first = false;

drv.selectLibrary(1);
drv.setMode(DRV2605_MODE_INTTRIG);
}
  drv.setWaveform(0, effect);  // play effect 
  drv.setWaveform(1, 0);       // end waveform

  // play the effect!
  drv.go();

  // wait a bit
  delay(500);

  effect++;
  if (effect > 117) effect = 1;
}
