//ESP32 LoRa Gateway Code//
//En el siguiente código, realice cambios en el SSID WiFi, la contraseña y la clave API de Thingspeak//
// 
#include <WiFi.h>
 
//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

   //Libraria para la pantalla Oled
     #include <U8g2lib.h>
     U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);
 
//define the pins used by the LoRa transceiver module

#define SCLK 5
#define MISO 19
#define MOSI 27
#define CS 18
#define RST 23
#define DIO 26
 
#define BAND 915E6    //433E6 for Asia, 866E6 for Europe, 915E6 for North America
 
 
// Replace with your network credentials
String apiKey = "ZIAIA0FNJK6MGOM0"; // Enter your Write API key from ThingSpeak
const char *ssid = "DmGv_2.4GHz"; // replace with your wifi ssid and wpa2 key
const char *password = "1$h1k1.22";
const char* server = "api.thingspeak.com";
 
WiFiClient client;
 
 
// Initialize variables to get and save LoRa data
int rssi;
String loRaMessage;
String b_densidad_polvo;
String b_ppm_real;
String readingID;


  int muestreo = 36; // Pin analógico para el pin Vo del sensor GP2Y10
  int IRED = 25; // Pin digital para el IRED
 
  // Tiempos constantes para el pulso de control del IRED
  int retardo_1 = 280;
  int retardo_2 = 40;
  int retardo_3 = 9680;
 
  // Variables auxiliares del programa
  int valor = 0;
  float ppm = 0;
  float voltaje = 0;
  float a_densidad_polvo = 0;
  float a_ppm_real = 0;
  int i=0;
 
// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "DENSIDAD")
  {
    return b_densidad_polvo;
  }
  else if(var == "CONCENTRACION")
  {
    return b_ppm_real;
  }
  else if (var == "RRSI")
  {
    return String(rssi);
  }
  return String();
  }



  void getReadings(){
    i=i+1; // Contador de pulsos de control
    // El IRED se va a activar con LOW y se desactiva con HIGH según las conexiones internas del sensor
    digitalWrite(IRED,LOW); // LED activado
    delayMicroseconds(retardo_1); // Retardo de 0,28ms
    valor = analogRead(muestreo); // Se muestrea el valor de Vo a través del pin analógico A0
    ppm = ppm + valor; // Media ponderada de Vo
    delayMicroseconds(retardo_2); // Retardo de 0,04ms
    digitalWrite(IRED,HIGH); // LED desactivado
    delayMicroseconds(retardo_3); // Retardo de 9,68ms
    // retardo_1 + retardo_2 + retardo_3 = 10ms
   
    // Fórmulas matemáticas para el cálculo de los valores del sensor GP2Y10
    voltaje = ppm/i*0.0049; // Voltaje en voltios (media de los valores ppm obtenidos)
    a_densidad_polvo = (0.17*voltaje-0.1)*1000; // Densidad de partículas de polvo en ug/m3
    a_ppm_real = (voltaje-0.0356)*120000; // Concentración de partículas de polvo en ppm
    if (a_ppm_real < 0)
      a_ppm_real = 0;
    if (a_densidad_polvo < 0 )
      a_densidad_polvo = 0;
    if (a_densidad_polvo > 500)
      a_densidad_polvo = 500;
   
    // Presentamos el voltaje a través del MONITOR SERIE
    
    }

   void setup() {
  Serial.begin(115200);
  int counter;
 
  //setup LoRa transceiver module
  LoRa.setPins(CS, RST, DIO); //setup LoRa transceiver module
 
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(2000);
  }
  if (counter == 10) {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  delay(2000);
 
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

    pinMode(IRED,OUTPUT); // Pin digital 25 como salida
    Serial.println(F("DETECTOR  DE PARTICULAS POLVO"));
    u8g2.begin(); 
}
 
 
  // Read LoRa packet and get the sensor readings
  void loop() 
   {
  
   getReadings();

   int packetSize = LoRa.parsePacket();
   if (packetSize)
   {
    Serial.print("Lora packet received: ");
    while (LoRa.available())    // Read packet
   {
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData); 
    
    
    int pos1 = LoRaData.indexOf('/');   
    int pos2 = LoRaData.indexOf('&');   
    readingID = LoRaData.substring(0, pos1);                        // Pedir readingID
    b_densidad_polvo = LoRaData.substring(pos1 +1, pos2);           // Pedir DENSIDAD
    b_ppm_real = LoRaData.substring(pos2+1, LoRaData.length());     // Pedir CONCENTRACION
   }
  
  rssi = LoRa.packetRssi();       // Get RSSI
  Serial.print(" with RSSI ");    
  Serial.println(rssi);
   }
 
  
   if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
   {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(a_densidad_polvo);
      postStr += "&field2=";
      postStr += String(a_ppm_real);
      postStr += "&field3=";
      postStr += String(b_densidad_polvo);
      postStr += "&field4=";
      postStr += String(b_ppm_real);
      postStr += "&field5=";
      postStr += String(rssi);
      
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

    // Presentamos el voltaje a través del MONITOR SERIE
    
    Serial.print("-> VOLTAJE A: ");
    Serial.print(voltaje); // Tres decimales
    Serial.print(" V");
   
    // Presentamos la densidad de partículas de polvo a través del MONITOR SERIE
    
    Serial.print("  -> DENSIDAD A: ");    
    Serial.print(a_densidad_polvo); // Tres decimales
    Serial.print(" ug/m3");
   
    // Presentamos la concentración de partículas de polvo a través del MONITOR SERIE

   Serial.print("  -> CONCENTRACION A: ");
   Serial.print(a_ppm_real); // Tres decimales
   Serial.println(" ppm");



     u8g2.clearBuffer();                          //Limpiar la memoria interna
     u8g2.setFont(u8g2_font_pxplusibmvga9_tr);    //Elija una fuente adecuada en https://github.com/olikraus/u8g2/wiki/fntlistall
     u8g2.drawStr(0,15,"Dust Density A: ");          //Escribe algo en la memoria interna
     u8g2.setCursor(0, 31);
     u8g2.print(a_densidad_polvo); 
     u8g2.sendBuffer();                           //Transferir la memoria interna a la pantalla

    
    //delay(30000);
    }
   
