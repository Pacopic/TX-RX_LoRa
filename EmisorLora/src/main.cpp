#include <Arduino.h>
#include "header.h"
#define HacerDebug = 1

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */ 

// declaración de variables y constantes
//#define  trama 6 //número de bytes que se enviaran por LORA, determina el tamaño del array payload.
uint8_t  tramaLora[] = {0,0,0,0,0,0}; //Array para la transmisión de datos LORA 
        // Identificador, valorMSB del sensor, valorLSB del sensor, estado

uint8_t counter = 0; //variable contador para saber el número de paquete enviado. 

uint16_t TiempoSleep = 5; //tiempo en segundos de deep sleep.
//Configuración ADC humedad
RTC_DATA_ATTR uint16_t  AirValue = 2700; //valor que indica el sensor midiendo el aire. 
RTC_DATA_ATTR uint16_t  WaterValue = 1130;  //Medida del sensor con máxima humedad, sumergido en agua. 
RTC_DATA_ATTR uint16_t  VBat_min = 0;  //Valor mínimo de tensión de batería. hay que ver el divisor que montamos.
RTC_DATA_ATTR uint16_t  VBat_max = 0xFFFF;  //Valor máximo de tensión de batería. Hay que ver el divisor que se monta.
uint8_t         humedad = 0;
uint8_t         VBat = 0;
// valores precedidos de "RTC_DATA_ATTR" se conservan después del despertar.
uint16_t TiempoEsperaRX = 5000;  //Tiempo de espera para recibir respuesta de la central.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
//==================================================================================================

void LoRa_Ini()//Inicialización Conexiones SPI dispositivo LoRa 
  {
    SPI.begin(SCK, MISO, MOSI, SS); //definición de los pines de comunicación con el módulo LORA
    //setup LoRa transceiver module
    LoRa.setPins(SS, RST, DIO0);
    
    if (!LoRa.begin(BAND)) //comprobación inicialización LoRa
        {
          Serial.println("Starting LoRa failed!");
          display.clearDisplay();
          display.setTextColor(WHITE);
          display.setTextSize(1);
          display.setCursor(0,0);
          display.print("Starting LoRa ");
          display.setCursor(0,20);
          display.print("failed:  Reset unit ");
          display.print(Id);  //identificador del dispositivo.
          display.display();
          while (1);
        }
          //Configuración Módulo LoRa para la transmisión
    LoRa.setSpreadingFactor(12);  // De 6 a 12. Indica la cantidad de datos redundantes que se envían en la transmisión. 
                                    // Mayor valor=>menor velocidad de transmisión de datos y mayor alcance.
                                    //Configuración potencia de radio
                                    //Valor por defecto: LoRa.setTxPower(17,PA_OUTPUT_PA_BOOST_PIN);
                                    //Most modules have the PA output pin conneted to PA_BOOSTER, gain 2-17
                                    //TTGO and some modules are conected to RFO_HF, gain 0-14
                                    //If your receiver RSSI is very Weak and little affected by a better antenna, change this!!
                        //  LoRa.setTxPower(14,PA_OUTPUT_RF_PIN); 
      //  LoRa.setTxPower(14,PA_OUTPUT_RFO_PIN); //probar esta instrucción
  
    Serial.println("LoRa Inicializacion OK!");
    display.setCursor(0,10);
    display.display();
    return;
  }
//==================================================================================================
void Oled_Ini() //reset pantalla OLED   
{
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  //Inicialización pantalla OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) 
  { // Address 0x3C for 128x32
    Serial.println(F("No se encontró SSD1306 (allocation failed)"));
    Serial.println("Reinicie el dispositivo");
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  /*display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  //Mensaje inicialización
  display.print("LORA SENDER ");
  display.setCursor(0,20);
  display.print("Dispositivo: ");
  display.print(Id);  //identificador del dispositivo.
  */
 display.display();
  return;
}
//====== presenta datos pantalla =============
void EscribeDisplay()
  {
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Granadal");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("Sector: ");
  display.print(Id);
  display.setCursor(0,30);
  display.print("Humedad:  ");
  //display.setCursor(60,30);
  display.print(humedad);
  display.setCursor(0,40);
  display.print("V Bat:  ");
  display.print(VBat);
  display.display();
  return;
  }
//==================================================================================================
//======== Función para leer los valores de ADC
uint8_t LeeValorADC(uint8_t ADC, uint16_t valormin, uint16_t valormax)  //Esta función lee los datos del ADC y devuelve el valor en un solo byte de acuerdo a los parámetros de calibración.
  {     //Argumentos: Puerto ADC a leer, Valor mínimo y valor máximo que puede tomar
    int16_t  aux =0;
    int16_t DatoADC[5]  ={0,0,0,0,0} ;
    uint8_t   ValorSensor = 0;
    uint8_t lectura=0;
    
               //En esta función haremos el cálculo de la mediana para obtener los dos datos únicos. 
    for (lectura=0; lectura <4; lectura++)
      {
      DatoADC[lectura] = analogRead(ADC);  //Guarda el valor del ADC en un array de 5 valores cada segundo.
      delay(200);
      }
  // calculamos la mediana del array y convertimos su valor a byte para ser enviado.
    for (lectura=0; lectura<4; lectura++)
      { //ordenamos de menor a mayor
        if (DatoADC[lectura]>DatoADC[lectura+1])
          {
            aux = DatoADC[lectura+1];
            DatoADC[lectura+1] = DatoADC[lectura];
            DatoADC[lectura]= aux;
            lectura = 0;
          }
      }
  ValorSensor = map(DatoADC[2], AirValue, WaterValue, 0, 255); //Pasamos el valor de humedad entre 0 y 255
  if(ValorSensor >= 255)
          { ValorSensor = 255;  }
  if(ValorSensor <=0)
          { ValorSensor = 0;    }
  return ValorSensor; 
  } 
  
//==================================================================================================
//====== Envia Datos LORA =============

void EnviaDatosLora()
{
  tramaLora[0] = Id;  //El primer byte a enviar es el identificador de dispositivo.
  tramaLora[1]= humedad;
  tramaLora[2]= VBat; //Valor de la tensión de batería. para saber en que estado se encuentra.
  tramaLora[3]= counter;
  tramaLora[4]= counter;
  tramaLora[5]= counter; 
  Serial.println("Trama LoRa TX: ");  //Enviamos datos a Lora y al puerto Serie.
    //Send LoRa packet to receiver
    Serial.print("size of tramalora: ");
  
      Serial.println(sizeof(tramaLora));
  LoRa.beginPacket(); //Se le puede dar un valor de encabezado. Quizás podríamos usarlo para enviar ahí el ID del dispositivo
        //devuelve un 1 cuando termina. Selecciona si se envía con Encabezado o no.
uint8_t tamanoTramaLora = sizeof(tramaLora);
  for (uint8_t t=0; t<tamanoTramaLora; t++)
    {
      LoRa.write(tramaLora[t]);     // La función devuelve el puntero de
      //envia los datos tal cual. su valor.
    } 
  LoRa.endPacket();
  
  for (uint8_t t=0; t<tamanoTramaLora; t++)
    {//envia los datos al puerto serie
      Serial.print("Dato ");
      Serial.print(t);
      Serial.print(" = ");
      Serial.println(tramaLora[t]); //enviamos el mismo dato al puerto serie.
    } 
Serial.println("finalizado envio datos Lora: Antes del return");
return;
}
//===================================================================
//==========Recibir dato por Lora
uint16_t RxDatoLora()
{
  uint16_t InicioRx = millis();
  uint16_t tiempoNuevaTransmision = 1; //tiempo para la nueva transmisión. si no se recibe nada, volverá a transmitir en un segundo. 
  String LoRaData ;
  while (millis()-InicioRx < TiempoEsperaRX)
  {
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
    if (Rx_Data[0]==Id) 
      {
        Serial.print("recibido mensaje para ID 3");
        tiempoNuevaTransmision = Rx_Data[1]; // devolvemos el valor del tiempo para la próxima transmisión.
      }
    else Serial.print("esperando mensaje para ID3");
    break; //sale del while de millis
  }
return tiempoNuevaTransmision; //Si no hemos recibido señal en un tiempo, reinicia y vuelve a emitir tras un segundo. Si recibimos un valor por parte del transmisor, se lo pasaremos a la función Dormir()
}
//==================================================================================================
void Dormir (int16_t TIME_TO_SLEEP)
{
  Serial.println("entrando en dormir");
  delay(1000);
  //Convertimos el tiempo de sueño que le hemos pasado en segundos a microsegundos para la función.
  uint64_t TS = TIME_TO_SLEEP * uS_TO_S_FACTOR; //Convertimos los segundos a microsegundos para pasarselo a la funcion WAKEUP.
  esp_sleep_enable_timer_wakeup(TS);  //Indicamos en microsegundos cuanto tiempo ha de dormir.
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON); // all RTC Peripherals are powered
  esp_deep_sleep_start();
  return;
}  
  //==================================================================================================

void setup() 
{  //Inicialización puerto serie
  Serial.begin(115200);
  Serial.println("Inicializando");
  Oled_Ini() ;  //Inicializa la pantalla OLED.LoRa_Ini();//Inicialización Conexiones SPI dispositivo LoRa 
  LoRa_Ini();
}  
void loop() //realmente no se ejecuta un loop  ya que el dispositivo se reinicia tras la función Dormir().
  {         //Lo hemos dejado así para separar lo que son las instrucciones de inicialización del programa. 
  humedad = LeeValorADC(PinSensor, AirValue, WaterValue);
  Serial.print("humedad: ");
  Serial.println(humedad);
  VBat = LeeValorADC(PinVBat, VBat_min, VBat_max );
  Serial.print("Vbat: ");
  Serial.println(VBat);
  EscribeDisplay();
  EnviaDatosLora();
  //  Recibir datos del maestro con siguiente emisión y potencia de recepción, por si necesita aumentarla
  counter++;  //Hay que pasar estaas variables a la memoria RTC si no queremos que se pierdan.
  Serial.println("iniciando dormir");
  delay(10);
  TiempoSleep = 10;// RxDatoLora();
  //ver nivel de señal con el que reciben al nodo para que ajuste la potencia de emisión
  //Si no hay recepción, incrementarla potencia de salida. si hay recepcion, disminuir hasta tener un valor dB adecuado
   Dormir(TiempoSleep);
  }