#include <SPI.h>              // include libraries
#include <GP2YDustSensor.h>

const uint8_t SHARP_LED_PIN = 25;   // Sharp Dust/particle sensor Led Pin
const uint8_t SHARP_VO_PIN = 39;    // Sharp Dust/particle analog out pin used for reading 

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, SHARP_LED_PIN, SHARP_VO_PIN);

void setup() {
  Serial.begin(115200);
  dustSensor.setBaseline(5.0); // set no dust voltage according to your own experiments
  //dustSensor.setCalibrationFactor(1.1); // calibrate against precision instrument
  dustSensor.begin();
}

void loop() {

  Serial.print("Dust density: ");
  Serial.print(dustSensor.getDustDensity());
  Serial.print(" ug/m3; Running average: ");
  Serial.print(dustSensor.getRunningAverage());
  Serial.println(" ug/m3");
  delay(1000);
}
