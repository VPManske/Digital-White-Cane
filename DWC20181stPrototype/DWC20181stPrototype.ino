/* uses i2c_t3.h works on Teensy LC and gives no errors.
*/

// #include <Adafruit_NeoPixel.h>
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
const boolean use_haptic_rtp = true;

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
 
 
  if ( use_haptic_rtp ) {
    // We are using 2V LRA
    
    drv.writeRegister8(DRV2605_REG_FEEDBACK, 0b10110110); // Register 0x1A 
    // Bit 7 1:LRA Mode
    // bits 6-4 0b011: 4x FB_BRAKE_FACTOR
    // bits 3-2 0b01: medium LOOP_GAIN
    // bits 1-0 0b10: LRA Mode 20x BEMF_GAIN
   
    
    drv.writeRegister8(DRV2605_REG_CONTROL1, 0x94);  // register 0x1B
     // at 205 Hz which means a half cycle is 2.439ms which is close to
    // (20*0.1) + 0.5 = 2.5ms 20dec is 0x14 or that with 0x80 for STARTUP_BOOST to get 0X94
    
    drv.writeRegister8(DRV2605_REG_CONTROL2, 0b01110101); // register 0x1C 
    // BIDIR_INPUT = 0 : Unidirectional input mode (requires closed loop)
    // BRAKE_STABILIZER = 1 : improve stability
    //  SAMPLE_TIME:3 which is 300us
    // BLANKING_TIME : 1 valid for most devices
    // IDISS_TIME : 1 valid for most devices
    drv.writeRegister8(DRV2605_REG_CONTROL3, 0b10001100); //0x1D 
    // NG_THRESHOLD: 2 (4 % default)
    // ERM_OPEN_LOOP:0 (Closed loop, doesn't matter, we aren't using ERM)
    // SUPPLY_COMP_DIS:0 Enabled
    // DATA_FORMAT_RTP:1 Unsigned (0 to 255, not -128 to +127)
    // LRA_DRIVE_MODE: 1 twice per cycle (more precise control)
    // N_PWM_ANALOG: 0 PWM Input (we aren't using PWM or Analog, but if we did...)
    // LRA_OPEN_LOOP: 0 Auto-resonance mode
   
    drv.writeRegister8(DRV2605_REG_CONTROL4,0x30);   // 
    // AUTO_CAL_TIME 1000ms - 1200ms
    // Not touching OTP
    
    drv.writeRegister8(DRV2605_REG_RATEDV, 81); // equation 3 of page 22 of DRV2605 datasheet
    // Note that SAMPLE_TIME is 300us and the
    // LRA frequency is 205 Hz
    // Rated voltage is 2.0V plugged into the equation gives us:
    // (.02071 * 81)/sqrt(1-(4*0.0003 + 0.0003)*205)
    // 2.015838


    drv.writeRegister8(DRV2605_REG_CLAMPV, 137); // equation 7 page 21
    drv.writeRegister8(DRV2605_REG_MODE,DRV2605_MODE_AUTOCAL); // Choose autocal mode
    drv.go(); // Run chosen mode
    Serial.print("Calibrating ");
    Serial.print(hapticId);
    long beginCal = millis();
    long nextDot = beginCal + 100;
    while(drv.stillGoing()) {
      if ( millis() > nextDot ) {
        Serial.print(" .");
        nextDot += 100;
        if ( nextDot > beginCal + 3000 ) {
          break;
        }
      }
    }
    Serial.print(" done after ");
    Serial.print(millis() - beginCal);
    Serial.println("ms.");
    drv.writeRegister8(DRV2605_REG_MODE, DRV2605_MODE_REALTIME);
    drv.go(); // Run chosen mode
  } else {
    drv.begin();
    drv.selectLibrary(1);

    // I2C trigger by sending 'go' command
    // default, internal trigger when sending GO command
    drv.setMode(DRV2605_MODE_INTTRIG);
  }
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
  uint32_t now = millis();
  while ( now < goal ) {
    delay(10);
    now = millis();
  }
}
// Values used at Cleveland Clinic hackathon
// int strengths[] = {254, 250, 240, 230, 220, 210, 200, 190, 180, 170, 160, 150, 140, 130, 120, 110, 100, 90, 0, 0};

int strengths[] = {255, 235, 215, 205, 195, 175, 155, 135, 115, 95, 75, 55, 50, 45, 40, 35, 30, 25, 20, 0};
void loop() {
  VL53L0X_RangingMeasurementData_t datum;
  VL53L0X_Error errCode;
  long startTime = millis();
  for (int i = 0; i < num_of_sensors; i ++ ) {
    errCode = tofs[i].getSingleRangingMeasurement(&datum,  false );
    int ledStat = (datum.RangeMilliMeter > 1000 );
    digitalWrite(leds[i], ledStat );
    Serial.print(i);
    Serial.print(": ");
    Serial.print( datum.RangeMilliMeter );
    Serial.print("  ");
    if ( datum.RangeMilliMeter < 2000 ) {
      if ( use_haptic_rtp ) {
        tcaselect(i);
        Serial.print(strengths[datum.RangeMilliMeter / 100]);
        drv.setRealtimeValue(strengths[datum.RangeMilliMeter / 100]);
        Serial.print(" ");
      } else {
     //   int effect = 0;
    //    if ( datum.RangeMilliMeter < 200 ) {
  //        effect = 47; // Buzz 1 - 100%
  //      } else if ( datum.RangeMilliMeter < 400 ) {
  //        effect = 48; // Buzz 2 - 80%
  //      } else if ( datum.RangeMilliMeter < 600 ) {
  //        effect = 49;  // Buzz 3 - 60%
  //      } else if ( datum.RangeMilliMeter < 800 ) {
  //        effect = 50;  // Buzz 4 - 40%
  //      } else if ( datum.RangeMilliMeter < 1000 ) {
  //        effect = 50;  // Buzz 5 - 20%
  //      }
  //      uint32_t now = millis();
        // Don't ask to start playing a new waveform until the last one is finished.
        // If the current waveform will finish in 100 ms or less, just wait.
  //      if ( now < lastEffectEnd[i] &&  now > (lastEffectEnd[i] - 100) ) {
   //       Serial.print("#");
 //         waitUntil(lastEffectEnd[i]);
 //         now = millis();
 //       }
        // If the current waveform is done, start the next one, otherwise
        // skip that one and get it on the next cycle.
        // I am assuming I will be able to speed up cycles so the skip function is useful.
        // As of this version, the loop tends to take 115 or more milliseconds. The
        // waveform takes around 210 milliseconds. So we never skip. (other waveforms take longer).
 //       if ( now >= lastEffectEnd[i] ) {
   //       tcaselect(i);
          /*  if ( lastEffect[i] != effect ) {
              tcaselect(i);
              drv.stop();
              lastEffect[i] = effect;
              drv.setWaveform(0, effect);
              drv.setWaveform(1, 0);
              drv.go();
            } */

   //       drv.stop();
 //         Serial.print("* ");
  //        lastEffectStart[i] = now;
  //        lastEffectEnd[i] = now + 210; // With 200 milliseconds, sometimes it got confused.
  //        lastEffect[i] = effect;
 //         drv.setWaveform(0, effect);
 //         drv.setWaveform(1, 0);
//drv.go();
       // }
      }

    } else {
      if ( use_haptic_rtp ) {
        tcaselect(i);
        drv.setRealtimeValue(0);
        Serial.print("00 ");
      }
    }
  }
  Serial.println(millis() - startTime);
  // checkHealth();
}


