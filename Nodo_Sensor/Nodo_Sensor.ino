
#include <SPI.h>              // include libraries
#include <LoRa.h>             //https://github.com/sandeepmistry/arduino-LoRa
#include "SSD1306.h"          //https://github.com/ThingPulse/esp8266-oled-ssd1306
//#include <WiFi.h>

#include <GP2YDustSensor.h>

////////////////////Pinout! Customized for TTGO LoRa32 V2.0 Oled Board!//////////////////
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

// Defineded Pines for SharpGP2Y10B //

const uint8_t SHARP_LED_PIN = 25;   // Sharp Dust/particle sensor Led Pin
const uint8_t SHARP_VO_PIN = 36;    // Sharp Dust/particle analog out pin used for reading 

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, SHARP_LED_PIN, SHARP_VO_PIN);
SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);    // INICIALIZACION DE PANTALLA

//////////////////////// CONFIG PLACA A //////////////
byte localAddress = 0xFF;                            // address of this device
byte destination = 0XAB;                              // destination to send to
int interval = 3000;                               // interval between sends
String message;                                    // send a message
String outgoing;                                   // outgoing message
byte msgCount = 0;                                 // count of outgoing messages
long lastSendTime = 0;                             // last send time
int contador = 0;                                  //
String IdNode = "A";                               //
float calibrate = 400;                            // Adjusted to ppm of Co2, clean air has 400ppm of Co2
/////////////////////////////////////////////////////

String CO2_level;
String AirQuality;

  void sendMessage(String outgoing) {
    
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
  printScreen();
  Serial.println("Sending message to :"+(destination));
  Serial.println(outgoing); 
  Serial.println();
  msgCount++;                           // increment message ID
  delay (2000);
}


String getReadingsDustDensity(){

  int   Reading = analogRead(36); 
  float Dustdensity = (dustSensor.getDustDensity());
  float Ppm = (dustSensor.getRunningAverage());
  

  if (Dustdensity <= 75)                               AirQuality = "Excelent";   // Arbitrary thresholds! or 'qualitative' or cannot be measured absolutely
  else if (Dustdensity > 76 && Dustdensity < 150)      AirQuality = "Very Good";
  else if (Dustdensity > 151 && Dustdensity < 300)     AirQuality = "Good";
  else if (Dustdensity > 301 && Dustdensity < 1500)    AirQuality = "Acceptable";
  else if (Dustdensity > 1501 && Dustdensity < 2998)   AirQuality = "Poor";
  else if (Dustdensity >= 2999)                        AirQuality = "_Very Poor";
  else                                                 AirQuality = "None";

  Serial.println("DustDensity Measurement:");
  Serial.println(String(Reading) + " " + String(Ppm) + "ug/m3 " + "Quality Air: "+AirQuality);
  
  String AQ = AirQuality.substring(0,1); 
  message = IdNode+AQ+"&"+String(Ppm,2)+"$"+String(Dustdensity,2);                //Convert Float values to one String
  return message;
    
}

String getReadingGas (){

  int   Reading = analogRead(39);                                   // Raw ADC reading ESP32 on VP or analogRead(39); on VN or analogRead(35); on GPIO 35, you choose!
  float sensorVoltage = Reading/1024.0 * 5.00;                      // Calibrated to measured sensor voltage using resitive divider
                                                                    // It's Important this comment for your proyect.
  float ppm = sensorVoltage * calibrate;                            // Adjusted to ppm of Co2, clean air has 400ppm of Co2, sensor output = 0.16v in clean air (0.16x2500=400)

  
  if      (ppm > 6000)                CO2_level = "Extreme";      
  else if (ppm > 1501 && 3000 > ppm)  CO2_level = "High";
  else if (ppm > 401 && 1500 > ppm)   CO2_level = "Moderate";
  else if (ppm < 400)                 CO2_level = "Low";
  else                                CO2_level = "None";
  
  Serial.println("CO2 Level");
  Serial.println(String(Reading) + " " + String(sensorVoltage) + "v " + String(ppm)+ "ppm " + " Co2 Level: "+ (CO2_level));
  
  String GS = CO2_level.substring(0,1); 
  message = IdNode+GS+"/"+String(ppm,2);                            //Convert Float values to one String
 return message;

}

void printScreen() {
        
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.setColor(BLACK);
  display.fillRect(0, 0, 127, 30);
  display.display();

  display.setColor(WHITE);
  display.drawString(0, 00, "LoRaWSN: " + String(localAddress));
  display.drawString(0, 10, " Node: " + IdNode
                          + " To: " + String(destination)
                          + " Air is: " + (AirQuality));
  display.drawString(0, 20, " CO2 Level: " + (CO2_level));
  display.display();
} 


void setup() {
  
  Serial.begin(115200);
  dustSensor.setBaseline(5.0);                                    // set no dust voltage according to your own experiments
  dustSensor.begin();
  
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);                                    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH);                                   // while OLED running, must set GPIO16 in high
  delay(1000);
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.clear();
  
  while (!Serial);
  Serial.println("TTGO LoRa32 V2.0 P2P");
  display.drawString(0, 00, "TTGO LoRa32 V2.0 P2P");
  display.display();
  
  LoRa.setPins(SX1278_CS, SX1278_RST, SX1278_DI0);// set CS, reset, IRQ pin
  if (!LoRa.begin(LORA_BAND))
  {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    display.drawString(0, 10, "LoRa init failed");
    display.drawString(0, 20, "Check connections");
    display.display();
    while (true);                       // if failed, do nothing
  }
  
  Serial.println("LoRa init succeeded.");
  display.drawString(0, 10, "LoRa init succeeded.");
  display.display();
  delay(1500);
  display.clear();
  display.display();
  
}

void loop() {
  if (millis() - lastSendTime > interval) {
    message = getReadingsDustDensity();
    sendMessage(message);
    
    
     if (millis() > 20000){            // time for warm the MQ135
       message = getReadingGas();
       sendMessage(message);
       
        }else{
          Serial.println("Wait 2-mins while sensor to warm-up!");
          display.drawString(0, 20, "MQ135 is warming up");
        }
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;     // 2-3 seconds
    }
}
