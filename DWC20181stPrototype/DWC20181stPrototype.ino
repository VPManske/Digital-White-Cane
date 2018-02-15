/* uses i2c_t3.h works on Teensy LC and gives no errors.
*/

#include <Adafruit_NeoPixel.h>
#include "Adafruit_VL53L0X_local.h"
#include "Adafruit_DRV2605.h"
#include "i2c_t3_local.h"


#define DEBUG false
Adafruit_DRV2605 drv;

int tofaddress = 1;

Adafruit_VL53L0X_local tofs[5];

class Colors
{
  public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};
Colors led_color[5];
int num_of_sensors = 5;


uint8_t xshut[5] = {0, 3,  14, 7, 9};
// uint8_t xshut[5] = {2, 3,  5, 7, 9};
uint8_t i2cAddr[5] = {2, 3,  5, 7, 9};
uint8_t lastEffect[5] = {0, 0, 0, 0, 0};
uint32_t lastEffectStart[5] = {0, 0, 0, 0, 0};
uint32_t lastEffectEnd[5] = {0, 0, 0, 0, 0};
const boolean use_haptic_rtp = true;

const int BRIGHTNESS = 50;
const int num_detectors = 5;
const int PIN = 21;
const boolean haptic_present = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, PIN, NEO_RGB + NEO_KHZ800);

// Adafruit_VL53L0X lox[5];



#define TCAADDR 0x71 // Why didn't we leave it at 0x70???

void tcaselect(uint8_t i) {
  if ( haptic_present ) {
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

    drv.writeRegister8(DRV2605_REG_CONTROL4, 0x30);  //
    // AUTO_CAL_TIME 1000ms - 1200ms
    // Not touching OTP

    drv.writeRegister8(DRV2605_REG_RATEDV, 81); // equation 3 of page 22 of DRV2605 datasheet
    // Note that SAMPLE_TIME is 300us and the
    // LRA frequency is 205 Hz
    // Rated voltage is 2.0V plugged into the equation gives us:
    // (.02071 * 81)/sqrt(1-(4*0.0003 + 0.0003)*205)
    // 2.015838


    drv.writeRegister8(DRV2605_REG_CLAMPV, 137); // equation 7 page 21
    drv.writeRegister8(DRV2605_REG_MODE, DRV2605_MODE_AUTOCAL); // Choose autocal mode
    drv.go(); // Run chosen mode
    Serial.print("Calibrating ");
    Serial.print(hapticId);
    long beginCal = millis();
    long nextDot = beginCal + 100;
    while (drv.stillGoing()) {
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
  Serial.println("xShut high");
  digitalWrite(xshut[tofId], HIGH);
  delay(2);
  Serial.print(": ");
  tofs[tofId].begin(i2cAddr[tofId], true, I2C_PINS_22_23);
  Serial.println("After begin");
  errCode = tofs[tofId].getSingleRangingMeasurement(&datum,  false );
  Serial.println("After first measurement");
  Serial.print( datum.RangeMilliMeter );
}
void init_tofs_haptic() { //

  long startTime = millis();
  Serial.print("Initialising tof sensors. ");
  for (int i = 0; i < num_of_sensors; i ++ ) {
    Serial.print(i);
    init_tof(i);
    Serial.print("  ");
    if ( haptic_present ) {
      init_haptic(i);
    }
  }
  Serial.println(millis() - startTime);
}

void setup() {
  Serial.begin(115200);

  delay(200);
  Serial.println("step one");
  // put your setup code here, to run once:
  // pinMode(2,HIGH);
  //  pinMode(3,LOW);
  strip.setBrightness(50);

  strip.begin();
  //  strip.show();
  Serial.println("step two");




  for (int i = 0; i < num_detectors; i++)
  {
    strip.setPixelColor(i, strip.Color(0, 0, 255, 50));
    Serial.println("step two point five");
  }
  Serial.println("step three");
  strip.show();
  Serial.println("step four");



  // for (int i = 0; i < num_detectors; i++)
  //{
  strip.setPixelColor(0, strip.Color(255, 0, 0, 50));
  strip.setPixelColor(1, strip.Color(0, 255, 0, 50));
  strip.setPixelColor(2, strip.Color(0, 0, 255, 50));
  strip.setPixelColor(3, strip.Color(255, 0, 0, 50));
  strip.setPixelColor(4, strip.Color(0, 255, 0, 50));
  strip.setPixelColor(5, strip.Color(0, 0, 255, 50));

  // }
  strip.show();
  delay(200);
  Serial.print("Still ");
  strip.setPixelColor(1, strip.Color(255, 0, 0, 50));
  strip.setPixelColor(2, strip.Color(0, 255, 0, 50));
  strip.setPixelColor(3, strip.Color(0, 0, 255, 50));
  strip.setPixelColor(4, strip.Color(255, 0, 0, 50));
  strip.setPixelColor(5, strip.Color(0, 255, 0, 50));
  strip.setPixelColor(0, strip.Color(0, 0, 255, 50));


  strip.show();
  delay(200);
  strip.setPixelColor(2, strip.Color(255, 100, 100, 50));
  strip.setPixelColor(3, strip.Color(100, 255, 100, 50));
  strip.setPixelColor(4, strip.Color(100, 100, 255, 50));
  strip.setPixelColor(5, strip.Color(255, 0, 0, 50));
  strip.setPixelColor(0, strip.Color(0, 255, 0, 50));
  strip.setPixelColor(1, strip.Color(0, 0, 255, 50));
  strip.show();
  delay(200);
  Serial.println("kicking");


  for (int i = 0; i < num_of_sensors; i ++ ) {
    pinMode(xshut[i], OUTPUT);
    digitalWrite(xshut[i], LOW);
  }

  Wire.begin();// Default i2c bus kludged in the library to be on pins 16 and 17

  Wire1.begin();
  //  const uint32_t i2cDefaultTimeout = 2100000; // (2.1 seconds).(Has to be longer than the longest haptic waveform)
  //  Wire.setDefaultTimeout(i2cDefaultTimeout);
  //  Wire1.setDefaultTimeout(i2cDefaultTimeout);
  init_tofs_haptic();
}

void rangeToLEDColor(int ledNo, int range) {

  if ( range < 50 ) {
    strip.setPixelColor(ledNo, strip.Color(0, 255, 0, 75));
  } else if ( range < 80 ) {
    strip.setPixelColor(ledNo, strip.Color(120, 255, 0, 50));
  } else if ( range < 110 ) {
    strip.setPixelColor(ledNo, strip.Color(240, 180, 0, 50));
  } else if ( range < 140 ) {
    strip.setPixelColor(ledNo, strip.Color(255, 50, 0, 50));
  } else if ( range < 170 ) {
    strip.setPixelColor(ledNo, strip.Color(255, 0, 100, 50));
  } else if ( range < 200 ) {
    strip.setPixelColor(ledNo, strip.Color(200, 0, 200, 50));
  } else if ( range < 230 ) {
    strip.setPixelColor(ledNo, strip.Color(100, 0, 255, 50));
  } else if ( range < 300 ) {
    strip.setPixelColor(ledNo, strip.Color(50, 50, 255, 50));
  } else if ( range < 600 ) {
    strip.setPixelColor(ledNo, strip.Color(50, 100, 255, 50));
  } else {
    strip.setPixelColor(ledNo, strip.Color(0, 0, 255, 50));
  }

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


    Serial.print(i);
    Serial.print(": ");
    Serial.print( datum.RangeMilliMeter );
    Serial.print("  ");
    if ( datum.RangeMilliMeter < 2000 ) {
      if ( use_haptic_rtp ) {
        tcaselect(i);
        rangeToLEDColor(i, datum.RangeMilliMeter);
        if ( haptic_present ) {
          Serial.print(strengths[datum.RangeMilliMeter / 100]);
          drv.setRealtimeValue(strengths[datum.RangeMilliMeter / 100]);
        }
        Serial.print(" ");
      }

    } else {
      if ( use_haptic_rtp ) {
        strip.setPixelColor(i, strip.Color(0, 0, 50, 50));
        tcaselect(i);
        if ( haptic_present ) {
          drv.setRealtimeValue(0);
        }
        Serial.print("00 ");
      }
    }
  }
  strip.show();
  Serial.println(millis() - startTime);
  // checkHealth();
}


