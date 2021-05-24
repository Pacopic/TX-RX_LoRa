#include <Arduino.h>
 //Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> //pantalla oled

//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

uint16_t soilMoistureValue = 0;
uint8_t PorcentajeHumedad=0;
#define trama 6

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;
uint8_t  tramaLora[trama] = {0,0,0,0,0,0}; //Array para la transmisión de datos LORA 
        // Identificador, valorMSB del sensor, valorLSB del sensor, estado
uint16_t humedad = 0;

//==== FUNCIONES  =======


void setup() 
{ 
  
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.print("Granadal");
  display.setTextSize(1);
  display.display();
  
  //initialize Serial Monitor
  Serial.begin(115200);

  Serial.println("LoRa Receiver Test");
  
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(12);  // De 6 a 12. Indica la cantidad de datos redundantes que se envían en la transmisión. 
                                    // Mayor valor=>menor velocidad de transmisión de datos y mayor alcance.
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,30);
  display.println("LoRa Inicializacion OK!");
  display.display();  
}

void loop() {
int t = 0;
  //try to parse packet
  uint8_t packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    //received a packet
  Serial.print("Received packet ");
    //read packet
    while (LoRa.available()) 
      {
      LoRaData = LoRa.readString(); //ver porque lo lee como string
      //LoRaData = LoRa.read();
      }
//LoRaData.getBytes(tramaLora,trama); //con esta instrucción no funciona
  uint8_t longitudLoRaData = sizeof(LoRaData); //ver longitud LoRaData para extraer los bytes
  uint8_t Rx_Data[longitudLoRaData]= {};
  LoRaData.getBytes(Rx_Data,longitudLoRaData-3);
  
   Serial.println("TramaLora: ");
   Serial.println(LoRaData); 
  Serial.print("packetSize: ");
   Serial.println(packetSize);
   
   for (uint8_t t=0; t<packetSize; t++)
    {
      Serial.println(Rx_Data[t]); 
    }
    
   
    //print RSSI of packet
    uint32_t rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");    
    Serial.println(rssi);

    display.clearDisplay();
  //Primera línea: Titulo
      display.setTextSize(2);
      display.setCursor(0,0);
      display.print("Granadal");
  //Segunda línea: identificador zona
      display.setTextSize(1);
      display.setCursor(0,20);
      display.print("Sensor: ");
      display.print(Rx_Data[0]);  //LoRaData);
      display.setCursor(0,30);
      display.print("Humedad: ");
      display.print(Rx_Data[1]);  //LoRaData);
      //display.print(humedad);
      display.setCursor(0,40);
      display.print("contador: ");
      display.print(Rx_Data[4]);  //LoRaData);
      display.setCursor(0,50);
      display.print("Nivel RSSI: ");
      display.print(rssi);
      display.display(); 

  }
}