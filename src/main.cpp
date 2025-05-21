#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include "Arduino.h"
#include "WiFi.h"
#include "SPIFFS.h"            
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include "esp_sleep.h" 
#include <DNSServer.h> 
#include <cstdint>
#include <ESPmDNS.h>
#include "ThingSpeak.h"

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
unsigned long lastUploadTime = 0; 
const unsigned long uploadInterval = 5 * 60 * 1000;


uint32_t totalBytes;
uint32_t usedBytes;

void setupDefaultRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("HTTP_GET chamado");
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "application/javascript");
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

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
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
void getSPIFFSFilesList() {
  if(SPIFFS.begin(true)) {
    File root = SPIFFS.open("/");
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
      Serial.println("SPIFFS Mount Failed");
    }
}


String getFormattedTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro";
  }

  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timestamp);
}

void upload_data_fb() {

  float temperature = dht_sensor.readTemperature();
  uint8_t humidity = static_cast<uint8_t>(std::min(std::max(dht_sensor.readHumidity(), 0.0f), 255.0f));
  uint8_t sensorValue = analogRead(PHOTO_SENSOR);
  float voltage = sensorValue * (3.3 / 4095.0); // Conversão para tensão (ESP32 usa 12 bits ADC)
  float lux = voltage * 95.6; // Ajuste com um fator experimental (valor pode variar)

  if (Firebase.ready() && signupOK){
    String timestamp = getFormattedTimestamp();

    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature) && Firebase.RTDB.setInt(&fbdo, "/sensors/humidity", humidity) && Firebase.RTDB.setString(&fbdo, "sensors/timestamp", timestamp)) {
      Serial.println("Dados de umidade e temperatura enviados com sucesso");
    }
     
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("timestamp", timestamp);
    json.set("lux", lux);

    Serial.println("Dados enviados:");
    Serial.print("Temperatura :");
    Serial.println(temperature);
    Serial.print("Humidade: ");
    Serial.println(humidity);
    Serial.print("Luminiscência ");
    Serial.println(lux);
    Serial.print("Às ");
    Serial.println(timestamp);
    
    if (Firebase.RTDB.pushJSON(&fbdo, "sensors_data/sensor1", &json)) {
      Serial.println("Json enviado com sucesso");
    }
  }
}

void setup(){

  Serial.begin(115200);
  getSPIFFSFilesList();
  
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

  totalBytes = SPIFFS.totalBytes();
  usedBytes = SPIFFS.usedBytes();
  
  float totalMB = (float)totalBytes / 1048576.0;
  float usedMB = (float)usedBytes / 1048576.0;
  float freeMB = (float)(totalBytes - usedBytes) / 1048576.0;

  Serial.printf("Espaço total (SPIFFS): %.2f MB\n", totalMB);
  Serial.printf("Espaço usado: %.2f MB\n", usedMB);
  Serial.printf("Espaço livre: %.2f MB\n", freeMB);

}


 
void loop(){
    if (millis() - lastUploadTime >= uploadInterval || lastUploadTime == 0) {
    lastUploadTime = millis();  

    upload_data_fb();
    Serial.print("Memória livre: ");
    Serial.print(esp_get_free_heap_size());
    Serial.println(" bytes");
  }
}



