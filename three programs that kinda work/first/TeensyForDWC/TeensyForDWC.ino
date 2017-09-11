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
void init_tofs() {
  VL53L0X_RangingMeasurementData_t datum;
  VL53L0X_Error errCode;
  long startTime = millis();
  Serial.print("Initialising tof sensors. ");
  for (int i = 0; i < num_of_sensors; i ++ ) {
    Serial.print(i);
    digitalWrite(xshut[i], HIGH);
    delay(2);
    Serial.print(": ");
    tofs[i].begin(i2cAddr[i], false, I2C_PINS_22_23);
    errCode = tofs[i].getSingleRangingMeasurement(&datum,  false );


    Serial.print( datum.RangeMilliMeter );
    Serial.print("  ");
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
  init_tofs();


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
    if ( datum.RangeMilliMeter < 100 ) {
      tcaselect(i);
      drv.begin();

      drv.selectLibrary(1);

      // I2C trigger by sending 'go' command
      // default, internal trigger when sending GO command
      drv.setMode(DRV2605_MODE_INTTRIG);
      int effect = 1;
      drv.setWaveform(0, effect);
      drv.setWaveform(1, 0);
      drv.go();
    }
  }
  Serial.println(millis() - startTime);

}

