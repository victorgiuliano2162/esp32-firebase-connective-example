#include <Arduino.h>
#include <WiFi.h>              
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>

#define WIFI_SSID "brisa-pitoco" 
#define WIFI_PASSWORD "153769ab" 
#define API_KEY "AIzaSyDLTaSJJSQiB-5vFgtSzGzoxSFlWDxDof8" 
#define DATABASE_URL "https://esp32-umidty-sensor-default-rtdb.firebaseio.com/" 
#define DHT_SENSOR_PIN 4 
#define DHT_SENSOR_TYPE DHT11 


DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;                    

void setup(){
  Serial.begin(115200);
  dht_sensor.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  configTime(-3 * 3600, 0, "pool.ntp.org");
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

void loop(){

  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();

  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
   
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/temperature", temperature)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.print("Temperature : ");
      Serial.println(temperature);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: 2 " + fbdo.errorReason());
      Serial.print("Temperature : ");
      Serial.println(temperature);
    }
   
    if (Firebase.RTDB.setFloat(&fbdo, "/sensors/humidity", humidity)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
      Serial.print("Humidity : ");
      Serial.print(humidity);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: 1 " + fbdo.errorReason());
      Serial.print("Humidity : ");
      Serial.print(humidity);
    }
  }
}

