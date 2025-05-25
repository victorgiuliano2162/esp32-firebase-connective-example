#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include "Arduino.h"
#include "WiFi.h"
#include "LittleFS.h"            
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include "esp_sleep.h" 
#include <DNSServer.h> 
#include <cstdint>
#include <ESPmDNS.h>
#include "ThingSpeak.h"
#include <LittleFS.h>

#define FIREBASE_API_KEY "AIzaSyDLTaSJJSQiB-5vFgtSzGzoxSFlWDxDof8" 
#define FIREBASE_URL "https://esp32-umidty-sensor-default-rtdb.firebaseio.com/" 
#define DHT_SENSOR_PIN 4 
#define DHT_SENSOR_TYPE DHT11 
#define PHOTO_SENSOR 32
#define LED_LIGHT 2 
#define SSID "brisa-pitoco" 
#define PASSWORD "153769ab" 
#define THINGSPEAK_CHANNEL_NUMBER 1
#define THINGSPEAK_API_KEY "SADLJKDMSA"


DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
AsyncWebServer server(80);
WiFiClient client;
WiFiManager wifiManager;

bool signupOK = false; 
uint64_t lastUploadTime = 0; 
uint64_t lastBoardUploadTime = 0;
const uint32_t uploadInterval = 5 * 60 * 1000;
const uint32_t uploadBoardInterval = 30 * 60 * 1000;
char contador = 5;

char* free_heap;
char* min_free_heap;

float total_memory;
float used_storage;

const char* getFormattedTimestamp() {
  static char timestamp[20];
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro";
  }

  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return timestamp;
}

void logMemory() {
  json.clear();

  char free_heap_str[10];
  char min_free_heap_str[10];
  
  snprintf(free_heap_str, sizeof(free_heap_str), "%.2f KB", ESP.getFreeHeap() / 1024.0);
  snprintf(min_free_heap_str, sizeof(min_free_heap_str), "%.2f KB", ESP.getMinFreeHeap() / 1024.0);
  
  json.set("free_heap", free_heap_str);
  json.set("min_free_heap", min_free_heap_str);
  json.set("timestamp", getFormattedTimestamp());

  if (Firebase.RTDB.pushJSON(&fbdo, "board_status", &json)) {
  Serial.println("Dados da placa enviados com sucesso");
  }
}

void setupDefaultRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("HTTP_GET chamado");
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js", "application/javascript");
  });
  Serial.println("Foi chamado HTTP");
}

void firebase_config() {
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_URL;
  config.token_status_callback;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Conectado ao firebase");
    signupOK = true;
  } else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void autoConnect() {
  delay(500);
  wifiManager.autoConnect();
}

void wifi_connect(const char* ssid, const char* passw) {
  IPAddress local_ip(192, 168, 0, 100);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(local_ip, gateway, subnet, dns);
  WiFi.begin(ssid, passw);
  delay(1000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  }


}

//Lista e imprime os itens gravados na memória da esp
void getLittleFSFilesList() {
  if(LittleFS.begin(true)) {
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while(file) {
      Serial.print("Arquivo ");
      Serial.print(file.name());
      Serial.print(" Tamanho: ");
      Serial.print(file.size());
      Serial.println(" bytes");
      file = root.openNextFile();
    } 
  } else {
      Serial.println("LittleFS Mount Failed");
    }
}

void upload_data_fb() {

  float temperature = dht_sensor.readTemperature();
  uint8_t humidity = static_cast<uint8_t>(std::min(std::max(dht_sensor.readHumidity(), 0.0f), 255.0f));
  uint8_t sensorValue = analogRead(PHOTO_SENSOR);
  float voltage = sensorValue * (3.3 / 4095.0); // Conversão para tensão (ESP32 usa 12 bits ADC)
  float lux = voltage * 95.6; // Ajuste com um fator experimental (valor pode variar)
  

  if (Firebase.ready() && signupOK){
    json.clear();
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature) && Firebase.RTDB.setInt(&fbdo, "/sensors/humidity", humidity) && Firebase.RTDB.setString(&fbdo, "sensors/timestamp", getFormattedTimestamp())) {
      Serial.println("Dados de umidade e temperatura enviados com sucesso");
    }
     
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("timestamp", getFormattedTimestamp());
    json.set("lux", lux);

    Serial.println("Dados enviados:");
    Serial.print("Temperatura :");
    Serial.println(temperature);
    Serial.print("Humidade: ");
    Serial.println(humidity);
    Serial.print("Luminiscência ");
    Serial.println(lux);
    Serial.print("Às ");
    Serial.println(getFormattedTimestamp());
    
    if (Firebase.RTDB.pushJSON(&fbdo, "sensors_data/sensor1", &json)) {
      Serial.println("Json enviado com sucesso");
    }
  }
}


void display_memory_usage() {
  total_memory = LittleFS.totalBytes();
  used_storage = LittleFS.usedBytes();
  
  total_memory = total_memory / 1048576.0;
  used_storage = (float)used_storage / 1048576.0;
  float free_memory = (total_memory - used_storage) / 1048576.0;

  Serial.printf("Espaço total (LittleFS): %.2f MB\n", total_memory);
  Serial.printf("Espaço usado: %.2f MB\n", used_storage);
  Serial.printf("Espaço livre: %.2f MB\n", free_memory);
}

void turn_on_log() {

  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.setString(&fbdo, "turn_on", getFormattedTimestamp())){
      Serial.println("Horário de inicialização enviado");
    }
  }
}
 

void setup(){

  Serial.begin(115200);
  getLittleFSFilesList();
  
  pinMode(PHOTO_SENSOR, INPUT_PULLUP);
  dht_sensor.begin();
  configTime(-3 * 3600, 0, "pool.ntp.org");
  
  wifi_connect(SSID, PASSWORD);
  if (!MDNS.begin("dhtreadings")) {
    Serial.println("Erro ao iniciar mDNS");
    return;
  }

  setupDefaultRoutes();
  firebase_config();
  server.begin();
  Serial.println("Servidor iniciado");

  ThingSpeak.begin(client);

  display_memory_usage();
  turn_on_log();

}

void loop(){

  if (millis() - lastUploadTime >= uploadInterval || lastUploadTime == 0) {
    lastUploadTime = millis();  
    upload_data_fb();
  }

  if(millis() - lastBoardUploadTime >= uploadBoardInterval || lastBoardUploadTime == 0) {
    lastBoardUploadTime = millis();
    logMemory();
  }
}



/*
Para implementação futura:
void saveSensorData(float temp, float hum) {
  // Abre o arquivo no modo append (adiciona ao final)
  File file = LittleFS.open("/dht_data.txt", FILE_APPEND);
  
  if (!file) {
    Serial.println("Falha ao abrir arquivo para escrita!");
    return;
  }
  
  // Obtém o timestamp atual
  String timestamp = getTimestamp();
  
  // Formata a linha de dados
  String dataLine = timestamp + "," + String(temp) + "," + String(hum) + "\n";
  
  // Escreve no arquivo
  if (file.print(dataLine)) {
    Serial.println("Dados salvos: " + dataLine);
  } else {
    Serial.println("Falha ao escrever no arquivo!");
  }
  
  file.close(); // Fecha o arquivo
}

String getTimestamp() {
  // Simples timestamp baseado no tempo de execução
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  return String(minutes) + "m" + String(seconds) + "s";
}

void listFileContent() {
  // Função para ler e exibir o conteúdo do arquivo
  Serial.println("\nConteúdo do arquivo:");
  
  File file = LittleFS.open("/dht_data.txt", FILE_READ);
  if (!file) {
    Serial.println("Arquivo não encontrado!");
    return;
  }
  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();
}
*/