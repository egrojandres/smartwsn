//ESP32 LoRa Sensor Node Code//
//Antes de cargar, asegúrese de hacer los cambios en la frecuencia de LoRa según la región.//

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
 
//packet counter
int readingID = 0;
 
int counter = 0;
String LoRaMessage = "";


  
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
  float b_densidad_polvo = 0;
  float b_ppm_real = 0;
  int i=0;
 
 
 
//Initialize LoRa module
void startLoRA()
{
  LoRa.setPins(CS, RST, DIO); //setup LoRa transceiver module
 
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) 
  {
    // Increment readingID on every new reading
    readingID++;
    Serial.println("Starting LoRa failed!"); 
  }
  Serial.println("LoRa Initialization OK!");
  delay(2000);
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
    b_densidad_polvo = (0.17*voltaje-0.1)*1000; // Densidad de partículas de polvo en ug/m3
    b_ppm_real = (voltaje-0.0356)*120000; // Concentración de partículas de polvo en ppm
    if (b_ppm_real < 0)
      b_ppm_real = 0;
    if (b_densidad_polvo < 0 )
      b_densidad_polvo = 0;
    if (b_densidad_polvo > 500)
      b_densidad_polvo = 500;
   
    // Presentamos el voltaje a través del MONITOR SERIE
    
    Serial.print("-> VOLTAJE A: ");
    Serial.print(voltaje); // Tres decimales
    Serial.print(" V");
   
    // Presentamos la densidad de partículas de polvo a través del MONITOR SERIE
    
    Serial.print("  -> DENSIDAD A: ");    
    Serial.print(b_densidad_polvo); // Tres decimales
    Serial.print(" ug/m3");
   
    // Presentamos la concentración de partículas de polvo a través del MONITOR SERIE

   Serial.print("  -> CONCENTRACION A: ");
   Serial.print(b_ppm_real); // Tres decimales
   Serial.println(" ppm");   
}
 
void sendReadings() {
  LoRaMessage = String(readingID) + "/" + String(b_densidad_polvo) + "&" + String(b_ppm_real) ;
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  
  Serial.print("Sending packet: ");
  Serial.println(readingID);
  readingID++;
  Serial.println(LoRaMessage);
}
 
void setup() {
  //initialize Serial Monitor
      pinMode(IRED,OUTPUT); // Pin digital 25 como salida
      Serial.begin(115200);
      Serial.println(F("DETECTOR  DE PARTICULAS POLVO"));
      startLoRA();
       u8g2.begin();                                //Se Inicializa la pantalla OLED
}
void loop() {
     
     u8g2.clearBuffer();                          //Limpiar la memoria interna
     u8g2.setFont(u8g2_font_pxplusibmvga9_tr);    //Elija una fuente adecuada en https://github.com/olikraus/u8g2/wiki/fntlistall
     u8g2.drawStr(0,15,"Dust Density B: ");          //Escribe algo en la memoria interna
     u8g2.setCursor(0, 31);
     u8g2.print(b_densidad_polvo); 
     u8g2.sendBuffer();                           //Transferir la memoria interna a la pantalla
 
     getReadings();
     sendReadings();

}
