/* uses i2c_t3.h works on Teensy LC and gives no errors.
*/
#include "Adafruit_VL53L0X_local.h"
#include "Adafruit_DRV2605.h"
#include "i2c_t3_local.h"

#define DEBUG false
Adafruit_DRV2605 drv;

int tofaddress = 1;

Adafruit_VL53L0X_local tofs[5];
int num_of_sensors = 5;

uint8_t leds[5] = {21, 20, 91, 18, 15};
uint8_t xshut[5] = {2, 3,  5, 7, 9};
uint8_t i2cAddr[5] = {2, 3,  5, 7, 9};
uint8_t lastEffect[5] = {0, 0, 0, 0, 0};
uint32_t lastEffectStart[5] = {0, 0, 0, 0, 0};
uint32_t lastEffectEnd[5] = {0, 0, 0, 0, 0};

#define TCAADDR 0x71 // Why didn't we leave it at 0x70???

void tcaselect(uint8_t i) {
  if (i > 7) return;
  int res;

  Wire.beginTransmission(TCAADDR);

  res = Wire.write(1 << i);
  if ( res != 1 ) {
    Serial.print("res not expected: ");
    Serial.println(res);
  }
  Wire.endTransmission();
}
void init_haptic(int hapticId) {
  tcaselect(hapticId);
  drv.begin();

  drv.selectLibrary(1);

  // I2C trigger by sending 'go' command
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
}
void init_tof(int tofId) {
  VL53L0X_RangingMeasurementData_t datum;
  VL53L0X_Error errCode;
  digitalWrite(xshut[tofId], HIGH);
  delay(2);
  Serial.print(": ");
  tofs[tofId].begin(i2cAddr[tofId], false, I2C_PINS_22_23);
  errCode = tofs[tofId].getSingleRangingMeasurement(&datum,  false );
  Serial.print( datum.RangeMilliMeter );
}
void init_tofs_haptic() { //

  long startTime = millis();
  Serial.print("Initialising tof sensors. ");
  for (int i = 0; i < num_of_sensors; i ++ ) {
    Serial.print(i);
    init_tof(i);
    Serial.print("  ");
    init_haptic(i);
  }
  Serial.println(millis() - startTime);
}

void setup() {
  // put your setup code here, to run once:
  // pinMode(2,HIGH);
  //  pinMode(3,LOW);

  for (int i = 0; i < num_of_sensors; i ++ ) {
    pinMode(xshut[i], OUTPUT);
    digitalWrite(xshut[i], LOW);
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
  }
  for (int j = 0; j < num_of_sensors; j++) {
    digitalWrite(leds[j], LOW);
    delay(100);
  }
  for (int j = 0; j < num_of_sensors; j++) {
    digitalWrite(leds[j], HIGH);
    delay(100);
  }
  Serial.begin(115200);


  Serial.println("step one");

  Wire.begin();// Default i2c bus kludged in the library to be on pins 16 and 17

  Wire1.begin();
//  const uint32_t i2cDefaultTimeout = 2100000; // (2.1 seconds).(Has to be longer than the longest haptic waveform)
//  Wire.setDefaultTimeout(i2cDefaultTimeout);
//  Wire1.setDefaultTimeout(i2cDefaultTimeout);
  init_tofs_haptic();


}
void checkHealth() {
  if ( Wire.getError() >= 2 || Wire1.getError() >= 2 ) {
    Serial.println("Start BUS RESET, THIS IS NOT SUPPOSED TO HAPPEN (except during testing");
    Wire.resetBus();
    Wire1.resetBus();
    setup();
    Serial.println("End BUS RESET, THIS IS NOT SUPPOSED TO HAPPEN (except during testing");
  }
}
void waitUntil(uint32_t goal) {
// This needs to deal with the clock wrapping.
  uint32_t now=millis();
  while( now < goal ) {
    delay(10);
    now = millis();
  }
}
void loop() {
  VL53L0X_RangingMeasurementData_t datum;
  VL53L0X_Error errCode;
  long startTime = millis();
  for (int i = 0; i < num_of_sensors; i ++ ) {


    errCode = tofs[i].getSingleRangingMeasurement(&datum,  false );
    int ledStat = (datum.RangeMilliMeter > 100 );
    digitalWrite(leds[i], ledStat );
    Serial.print(i);
    Serial.print(": ");
    Serial.print( datum.RangeMilliMeter );
    Serial.print("  ");
    if ( datum.RangeMilliMeter < 1000 ) {

      int effect = 0;
      if ( datum.RangeMilliMeter < 200 ) {
        effect = 47; // Buzz 1 - 100%
      } else if ( datum.RangeMilliMeter < 400 ) {
        effect = 48; // Buzz 2 - 80%
      } else if ( datum.RangeMilliMeter < 600 ) {
        effect = 49;  // Buzz 3 - 60%
      } else if ( datum.RangeMilliMeter < 800 ) {
        effect = 50;  // Buzz 4 - 40%
      } else if ( datum.RangeMilliMeter < 1000 ) {
        effect = 50;  // Buzz 5 - 20%
      }
      uint32_t now = millis();
      // Don't ask to start playing a new waveform until the last one is finished.
      // If the current waveform will finish in 100 ms or less, just wait.
      if ( now < lastEffectEnd[i] &&  now > (lastEffectEnd[i]-100) ) {
        Serial.print("#");
        waitUntil(lastEffectEnd[i]);
        now =millis();
      }
      // If the current waveform is done, start the next one, otherwise
      // skip that one and get it on the next cycle.
      // I am assuming I will be able to speed up cycles so the skip function is useful.
      // As of this version, the loop tends to take 115 or more milliseconds. The
      // waveform takes around 210 milliseconds. So we never skip. (other waveforms take longer).
      if ( now >= lastEffectEnd[i] ) {
        tcaselect(i);
        /*  if ( lastEffect[i] != effect ) {
            tcaselect(i);
            drv.stop();
            lastEffect[i] = effect;
            drv.setWaveform(0, effect);
            drv.setWaveform(1, 0);
            drv.go();
          } */
  
        drv.stop();
        Serial.print("* ");
        lastEffectStart[i] = now;
        lastEffectEnd[i] = now + 210; // With 200 milliseconds, sometimes it got confused.
        lastEffect[i] = effect;
        drv.setWaveform(0, effect);
        drv.setWaveform(1, 0);
        drv.go();
      }

    }
  }
  Serial.println(millis() - startTime);
  // checkHealth();
}

