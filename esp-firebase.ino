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
#define PHOTO_SENSOR 32


DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;                     //since we are doing an anonymous sign in 

void setup(){
  Serial.begin(115200);
  pinMode(PHOTO_SENSOR, INPUT_PULLUP);
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

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
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
  unsigned int sensorValue = analogRead(PHOTO_SENSOR);
  float voltage = sensorValue * (3.3 / 4095.0); // Conversão para tensão (ESP32 usa 12 bits ADC)
  float lux = voltage * 95.6; // Ajuste com um fator experimental (valor pode variar)


  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 300000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
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
      Serial.println(humidity);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: 1 " + fbdo.errorReason());
      Serial.print("Humidity : ");
      Serial.println(humidity);
    }
    
    String timestamp = getFormattedTimestamp();
    json.set("temperature", temperature);
    json.set("humidity", humidity);
    json.set("timestamp", timestamp);
    //String newPath = "sensors/" + timestamp;
    Firebase.RTDB.setString(&fbdo, "sensors/timestamp", timestamp);
    //Firebase.RTDB.pushJSON(&fbdo, newPath, &json);
    if (Firebase.RTDB.pushJSON(&fbdo, "sensors_data/sensor1", &json)) {
      Serial.println("Dados pushados com sucesso em: sensors_data/sensor1");
    } else {
      Serial.println("Erro ao push JSON: " + fbdo.errorReason());
    }
    json.clear();
    json.set("lux", lux);
    json.set("timestamp", timestamp);
    Firebase.RTDB.pushJSON(&fbdo, "sensors_data/sensor2", &json);
    Serial.println(sensorValue);
    Serial.print("Luminiscência: ");
    Serial.println(lux);
  }
}