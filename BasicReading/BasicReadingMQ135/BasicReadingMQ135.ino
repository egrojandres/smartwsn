
#include "SH1106.h"               // See https://github.com/squix78/esp8266-oled-ssd1306 or via Sketch/Include Library/Manage Libraries
#include <SPI.h>              // include libraries
#include <LoRa.h>             //https://github.com/sandeepmistry/arduino-LoRa

#define SX1278_SCLK  5    // GPIO5  -- SX1278's SCLK
#define SX1278_MISO 19    // GPIO19 -- SX1278's MISO
#define SX1278_MOSI 27    // GPIO27 -- SX1278's MOSI
#define SX1278_CS   18    // GPIO18 -- SX1278's CS
#define SX1278_RST  23    // GPIO14 -- SX1278's RESET
#define SX1278_DI0  26    // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define OLED_SDA    21    // GPIO21  -- OLED'S SDA
#define OLED_SCL    22    // GPIO22  -- OLED's SCL Shared with onboard LED! :(
#define OLED_RST    16    // GPIO16  -- OLED's VCC?
#define LORA_BAND   915E6 // LoRa Band (America)
#define OLED_ADDR   0x3c  // OLED's ADDRESS

SH1106 display(0x3c, SDA, SCL);   // OLED display object definition (address, SDA, SCL)

String message;

void setup() {
  Serial.begin(115200);
  Serial.println("Wait 2-mins or more for sensor to warm-up!");
 
  Wire.begin(SDA, SCL, 100000);       // (sda,scl,bus_speed) Start the Wire service for the OLED display using pin=D4 for SCL and Pin-D3 for SDA
   
  display.init();                     // Initialise the display
  display.flipScreenVertically();     // In my case flip the screen around by 180°
  display.setContrast(255);           // If you want turn the display contrast down, 255 is maxium and 0 in minimum, in practice about 128 is OK
}

void loop() {
  int   Reading       = analogRead(39);          // Raw ADC reading ESP8266
  //int   Reading       = analogRead(36);        // Raw ADC reading ESP32 on VP or analogRead(39); on VN or analogRead(35); on GPIO 35, you choose!
  float sensorVoltage = Reading/1024.0 * 5.00;   // Calibrated to measured sensor voltage using resitive divider
  float ppm           = sensorVoltage * 400.0;  // Adjusted to ppm of Co2, clean air has 400ppm of Co2, sensor output = 0.16v in clean air (0.16x2500=400)
  Serial.print(String(Reading) + " " + String(sensorVoltage) + "v " + String(ppm) + "ppm Co2  ");
  if      (ppm > 6000) message = "Extreme";   // Arbitrary thresholds! or 'qualitative' or cannot be measured absolutely
  else if (ppm > 3000) message = "High";
  else if (ppm > 1500) message = "Moderate";
  else if (ppm > 500)  message = "Low";
  else                 message = "None";
  Serial.println(message);
  display.clear();
  display.setFont(ArialMT_Plain_16);  // Set the Font size LARGE
  display.drawString(20, 10, String(ppm,0));
  display.setFont(ArialMT_Plain_10);  // Set the Font size SMALL
  display.drawString(65,10,"ppm Co2");
  display.setFont(ArialMT_Plain_16);  // Set the Font size LARGE
  display.drawString(30, 30, message);
  display.display();
  delay(500);
}
