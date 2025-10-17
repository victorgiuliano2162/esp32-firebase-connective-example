# Exemplo de conexão IOT com banco o dados NoSQL Firebase - Realtime Database

## Hardware utilizado

* **Esp-WROOM-32**: Microcontroladora responsável pelo gerenciamento dos dados e conexão com o banco de dados;
* **DHT22**: Sensor de úmidade e temperatura;

## Estrutura de funcionanmento

* A microcontroladora se conecta à rede wifi;
* A microcontroladora inicia o sensor;
* A microcontroladora coleta os dados;
* A microcontroladora faz o envio dos dados para o banco de dados;
* A página web já estava disponível a partir do momento da conexão da microcontroladora à rede wifi;
* A página web faz uma requisição ao banco de dados;
* Os dados retornados são acessíveis ao usuário no link 192.168.0.100;

## Vídeo com exemplo de funcionamento
* https://photos.app.goo.gl/pAaGExc3iFkY818B7 

## Outras técnologias utilizadas

* **`C++ e javascript`**: liguagens de programação que possibilitaram a criação do projeto;
* **`PlaformIO`**: Framework responsável pela intereção com dispositivos IOT e compilação do código;
* **`Arduino`**: Framework com diversas função úteis para gerenciar IOT e fluxos de programa;

## Bibliotecas utlizadas

### 📚 Bibliotecas utilizadas

* **`WiFiManager.h`**: Gerencia conexão Wi-Fi via portal de configuração automático.  
* **`ESPAsyncWebServer.h`**: Cria servidores web assíncronos para ESP32 com melhor desempenho.  
* **`Arduino.h`**: Inclui definições básicas para programação em Arduino/ESP.  
* **`WiFi.h`**: Gerencia conexões Wi-Fi no ESP32.  
* **`LittleFS.h`**: Sistema de arquivos embutido para armazenar dados na flash.  
* **`TokenHelper.h`**: Auxilia na criação e gerenciamento de tokens JWT para Firebase.  
* **`RTDBHelper.h`**: Facilita operações com o Firebase Realtime Database.  
* **`DHT.h`**: Permite leitura de sensores de temperatura/umidade como DHT11 e DHT22.  
* **`esp_sleep.h`**: Controla modos de economia de energia e sono profundo no ESP32.  
* **`DNSServer.h`**: Permite redirecionar requisições DNS, útil para captive portals.  
* **`cstdint`**: Biblioteca padrão C++ com tipos de dados inteiros com tamanho fixo.  
* **`ESPmDNS.h`**: Adiciona suporte para mDNS (nome local de dispositivos na rede).  
* **`ThingSpeak.h`**: Envia dados para a plataforma de IoT ThingSpeak.  
* **`Firebase_ESP_Client.h`**: Permite autenticar, ler e escrever dados no Firebase via ESP. 
