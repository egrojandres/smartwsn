
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <WiFi.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

#define SCLK 5
#define MISO 19
#define MOSI 27
#define CS 18
#define RST 23
#define DIO 26
#define BAND 915E6        //433E6 for Asia, 866E6 for Europe, 915E6 for North America
#define OLED_ADDR   0x3c  // OLED's ADDRESS


// Replace with your network credentials

String apiKey = "****************";         // Enter your Write API key from ThingSpeak
const char *ssid = "YOUR_SSID";             // replace with your wifi ssid and wpa2 key
const char *password = "YOUR_PASWORD";
const char* server = "api.thingspeak.com";
 
WiFiClient client;


String message;

//String Dustdensity ="";
//String ppm ="";
String AdustDensity;
String BdustDensity;
String CdustDensity;
String DdustDensity;
String co2;
String Aco2;
String Bco2;
String Cco2;
String Dco2;

byte msgCount = 0;              // count of outgoing messages
byte localAddress = 0xAB;       // address of this device
//byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;          // last send time
int interval = 2000;            // interval between sends

void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);

  Serial.println("Nodo Maestro");
  
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(CS, RST, DIO);// set CS, reset, IRQ pin
  int counter;
  
   if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  
  Serial.println("LoRa init succeeded");

  // Connect to Wi-Fi network with SSID and password
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("WiFi connected.");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  u8g2.begin(); 
}   
void loop() {
 
    onReceive(LoRa.parsePacket());

  
}

void onReceive(int packetSize) { 
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  

  String incoming = "";
  
  while (LoRa.available()) {
  incoming += (char)LoRa.read();
  }
    
 if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

    if (incoming.charAt(2) == '$'){
      Serial.print ("Sensor Sharp: ");
      String Dustdensity = incoming.substring(3,incoming.length());
      Serial.println (Dustdensity);
        if (incoming.charAt(0) == 'A') {
        AdustDensity = Dustdensity;
        }else if (incoming.charAt(0) == 'B') {
          BdustDensity = Dustdensity;
            }else if (incoming.charAt(0) == 'C') {
            CdustDensity = Dustdensity;
              }else if (incoming.charAt(0) == 'D') {
              DdustDensity = Dustdensity;
                }else Serial.println("Sharp Load Fail");
        
      
      }else if(incoming.charAt(2) == '&'){
        Serial.print("Sensor MQ135: ");
        String co2= incoming.substring(3,incoming.length());
        Serial.println(co2);
        if (incoming.charAt(0) == 'A') {
          Aco2 = co2;
          }else if (incoming.charAt(0) == 'B') {
            Bco2 = co2;
            }else if (incoming.charAt(0) == 'C') {
              Cco2 = co2;
              }else if (incoming.charAt(0) == 'D') {
                Dco2 = co2;
                }else  (Serial.println("MQ135 Load fail"));
        
      }//else (Serial.println("Error de lectura"));

      delay (1500);

if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
   {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(AdustDensity);
      postStr += "&field2=";
      postStr += String(Aco2);
      postStr += "&field3=";
      postStr += String(BdustDensity);
      postStr += "&field4=";
      postStr += String(Bco2);
      postStr += "&field5=";
      postStr += String(CdustDensity);
      postStr += "&field6=";
      postStr += String(Cco2);
      postStr += "&field7=";
      postStr += String(DdustDensity);
      postStr += "&field8=";
      postStr += String(Dco2);
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
      delay (2000);
      }
}
