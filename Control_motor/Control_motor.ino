#include <Wire.h>
#include "Adafruit_DRV2605_mux_ctrl.h"

Adafruit_DRV2605 drv;
int address=0xe1;
void setup() {
  Serial.begin(9600);
  Serial.println("DRV test");
  pinMode(13, OUTPUT); 
 
 
  drv.begin();
  
  drv.selectLibrary(1);
  
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG); 
}

uint8_t effect = 1;

void loop() {
  Serial.print("Effect #"); Serial.println(effect);
  digitalWrite(13, HIGH);  
  delay(30);
  digitalWrite(13, LOW);  
  delay(30);
  
  // set the effect to play
  drv.setWaveform(0, effect);  // play effect 
  drv.setWaveform(1, 0);       // end waveform

  // play the effect!
  drv.go();

  // wait a bit
  delay(500);

  effect++;
  if (effect > 117) effect = 1;
}
