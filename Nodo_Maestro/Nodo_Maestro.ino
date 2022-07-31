#include <SPI.h>              // include libraries
#include <LoRa.h>             //https://github.com/sandeepmistry/arduino-LoRa
#include "SSD1306.h"          //https://github.com/ThingPulse/esp8266-oled-ssd1306
#include <WiFi.h>

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

SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);    // INICIALIZACION DE PANTALLA

//////////////////////// CONFIG PLACA A //////////////
byte localAddress = 8;                            // address of this device
byte destination = 18;                              // destination to send to
int interval = 3000;                               // interval between sends
String message = "";                               // send a message
String outgoing;                                   // outgoing message
byte msgCount = 0;                                 // count of outgoing messages
long lastSendTime = 0;                             // last send time
int contador = 0;                                  //
String IdPlaca = "A";                              //
/////////////////////////////////////////////////////

/////////////////////// CREDENCIALES DE RED ///////////////////////////////

int rssi;
String apiKey = "ZIAIA0FNJK6MGOM0";             // Enter your Write API key from ThingSpeak
const char *ssid = "MANUEL";                    // replace with your wifi ssid and wpa2 key
const char *password = "Negro1967";             // PASSWORD WiFi
const char* server = "api.thingspeak.com"; 
WiFiClient client; 


// Variables del código

String Dustdensity = "";
String Ppm = "";
String GasMeasurement = "";
String AdustDensity = "";
String AgasMeasurement = "";
String BdustDensity = "";
String BgasMeasurement = "";
String CdustDensity = "";
String CgasMeasurement = "";


void connectWiFi(){
//Conéctese a la red Wi-Fi con SSID y contraseña
      Serial.print("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    //Imprime la dirección IP local e inicia el servidor web
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  
      }

void printScreen() {
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.setColor(BLACK);
  display.fillRect(0, 0, 127, 30);
  display.display();
  display.setColor(WHITE);
  display.drawString(0, 00, "LoRaWSN: " + String(localAddress));
  display.drawString(0, 10, "Node: Maester");                  
  display.drawString(0, 20, " N: " + String(msgCount));
  display.display();
  
}

// Revisar función, debe separar el valor de dusttdendity y valor de gas por aparte.

void assignValues (String incommingText){

  int pos1;
  
  if (incommingText.indexOf('&')) {
  pos1 = incommingText.indexOf('&');
  Dustdensity = incommingText.substring(pos1,incommingText.length());
  
  if      (incommingText.charAt(0) == 'A') AdustDensity = Dustdensity;
  else if (incommingText.charAt(0) == 'B') BdustDensity = Dustdensity;
  else if (incommingText.charAt(0) == 'C') CdustDensity = Dustdensity;
  
  
  }else if (incommingText.indexOf('/')) {
  pos1 = incommingText.indexOf('/');
  GasMeasurement = incommingText.substring(pos1,incommingText.length());

  if      (incommingText.charAt(0) == 'A') AgasMeasurement = GasMeasurement;
  else if (incommingText.charAt(0) == 'B') BgasMeasurement = GasMeasurement;
  else if (incommingText.charAt(0) == 'C') CgasMeasurement = GasMeasurement;
  
  }else Serial.println("data reading error"); 

}


void onReceive(int packetSize) {
  
  if (packetSize == 0) {
  Serial.println("error");  
  return;          // if there's no packet, return
  }
  printScreen();
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  String incoming = "";                 // payload of packet
  String variable = "";


  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    incoming = "message length error";
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    incoming = "message is not for me";
    return;                             // skip rest of function
  }

  assignValues (incoming);

  display.setColor(BLACK);
  display.fillRect(0, 32, 127, 61);
  display.display();

  display.setColor(WHITE);
  display.drawLine(0,31,127,31);
  display.drawString(0, 32, "Rx PKT: " + incoming);
  display.drawString(0, 42, "RSSI: " + String(LoRa.packetRssi())
                          + " SNR: " + String(LoRa.packetSnr()));
  display.drawString(0, 52, "FR:"  + String(sender)
                          + " LN:" + String(incomingLength)
                          + " ID:" + String(incomingMsgId));
  display.display();

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  delay(1000);
}

void setup() {
  
Serial.begin(115200);
  
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);                    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH);                   // while OLED running, must set GPIO16 in high
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
  LoRa.setPins(SX1278_CS, SX1278_RST, SX1278_DI0); // set CS, reset, IRQ pin

  if (!LoRa.begin(LORA_BAND)) {                    // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    display.drawString(0, 10, "LoRa init failed");
    display.drawString(0, 20, "Check connections");
    display.display();
    while (true);                                 // if failed, do nothing
  }

  //LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa init succeeded.");
  display.drawString(0, 10, "LoRa init succeeded.");
  display.display();
  delay(1500);
  display.clear();
  display.display();

  //Connect to Wifi Network with SSID and Password
      Serial.print("Connecting to ");
      Serial.println(ssid);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    //Print the local IP address and start the web server
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

}

void loop() {
  if (millis() - lastSendTime > interval) {
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;     // 2-3 seconds
    LoRa.receive();                     // go back into receive mode
  }
  int packetSize = LoRa.parsePacket();
  if (packetSize) { onReceive(packetSize);  
  }
  if (client.connect(server, 80)){ // "184.106.153.149" or api.thingspeak.com
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += AdustDensity;
      postStr += "&field2=";
      postStr += AgasMeasurement;
      postStr += "&field3=";
      postStr += BdustDensity;
      postStr += "&field4=";
      postStr += BgasMeasurement;
      postStr += "&field5=";
      postStr += CdustDensity;
      postStr += "&field6=";
      postStr += CgasMeasurement;
      postStr += "&field7=";
      postStr += CgasMeasurement;
      postStr += "\r\n\r\n\r\n\r\n";
    
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
 
      }    
      delay(5000); //The data is uploaded to ThinkSpeak after an interval of 5s.
      }
