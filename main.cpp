#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>
#include <esp_now.h>

// Configuración de la dirección IP de la NodeMCU (cliente)
IPAddress clientIP(192, 168, 1, 100);  // Reemplaza con la IP de la NodeMCU
IPAddress serverIP(192, 168, 1, 101);  // Reemplaza con la IP de la Pine64
IPAddress dns(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
int serverPort = 8080;  // Puerto en el que el servidor (Pine64) está escuchando
byte mac[] = { 0xC4, 0xDD, 0x57, 0xC8, 0xD9, 0xB0 };
String dataToSend="";
// Define los pines SPI para la NodeMCU y el pin de selección de esclavo (CS) para el W5500
const int pinCS = 5;  // Pin de selección de esclavo (CS) para el W5500

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// direccion mac de la nodemcu para la esp-now
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xCA, 0x38, 0xD0};
// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  byte mensaje[80];
  int dataLen;
} struct_message;

typedef struct struct_ethernet {
  byte mensaje[80];
  int dataLen;
} struct_ethernet;

// Create a struct_message called myData
struct_message myDataSen;

// Create a struct_message called myData
struct_message myDataRec;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  //client.println("Estoy dentro de recivir");
  if (len <= sizeof(myDataRec.mensaje)){
    int dataLenToSend = len < 10 ? len : 10;  
    memcpy(myDataRec.mensaje, incomingData, dataLenToSend);
    myDataRec.dataLen=dataLenToSend;
    if (client.connected()) {
      for (int i = 0; i < dataLenToSend; i++) {
        dataToSend += (char)myDataRec.mensaje[i];
      }
      client.println(dataToSend);
    }
  } else {
    Serial.println("Received data too long for buffer");
  }
}

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  SPI.begin();
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(pinCS);   
// Configura la dirección IP y la conexión Ethernet
  Ethernet.begin(mac, clientIP, dns, gateway);
  delay(1000);  // Espera a que la conexión se establezca
  // Conecta al servidor (Pine64)
  if (client.connect(serverIP, serverPort)) {
    //Serial.println("Conectado al servidor");
    // Envía datos al servidor
    //client.println("Hola Mundo desde NodeMCU");
    //client.stop();  // Cierra la conexión
  } else {
    client.println("Error de conexión");
  }
  // Open serial communications and wait for port to open:

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  //Este apartado del peer es util para enviar el paquete
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Maneja el envío de datos al servidor (Pine64)
  if (client.connected()) {
    //client.println(dataToSend);
  }
  // Maneja la recepción de datos del servidor y muestra en el terminal serie
  static char receivedData[128];
  static int dataIndex = 0;
  //const char *myString="Esto es una prueba";
  //size_t str_len = strlen(myString);
  //uint8_t *prueba = (uint8_t *)myString;
  //esp_err_t result = esp_now_send(broadcastAddress, prueba, str_len);
  delay(1000);
}