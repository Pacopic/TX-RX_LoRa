//Rellenar estos campos para el dispositivo

#define Id  3 //identificador de dispositivo transmisor. Hasta 256 dispositivos
//Conexionado sensor Humedad
#define PinSensor 36   //ADC1_0  selección del pin de conexión del sensor de humedad. 
#define PinVBat 37   //ADC1_1 Selección del pin de conexión del sensor de voltaje de batería.

//Selección de frecuencia de funcionamiento según zona geográfica
//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//se definen las diferentes conexiones entre los distintos dispositivos de la placa. 
//Pines usados por el dispositivo LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26


//Conecionado pantalla OLED 
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//ver también el archivo de conexionado del dispositivo.

#define ADC1_0 36
#define ADC1_1 37

