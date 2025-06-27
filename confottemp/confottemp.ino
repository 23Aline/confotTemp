#include "DHT.h"
#include <stdio.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configurações WiFi
const char *ssid = "ESP32";
const char *password = "12345678";
const char *serverUrl = "https://conforttemp-backend.onrender.com/api/hardware/1";

// Configuração do DHT22
#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Definição dos pinos
const int led1 = 26;    // Azul - desligado
const int Relay = 32;   // Vermelho - ligado
const int ledPino = 25;  // LED de movimento
const int pirPino = 27; // Sensor PIR

unsigned long tempo = 0; /*Variável utilizada para salvar o valor do millis*/
unsigned long auxiliar_sensor = 0; /*Variável auxiliar*/
unsigned long auxiliar_wifi = 0; /*Variável auxiliar*/
float h = 0.0;
float t = 0.0;
bool s = false;
bool motion_anterior = false;

void connectToWiFi()
{
  Serial.print("Conectando-se a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20)
  {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi conectado.");
    Serial.print("Endereço de IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("\nFalha ao conectar no WiFi.");
  }
}

void envia_dados(){
// Enviar dados para API
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // JSON otimizado
    StaticJsonDocument<128> doc;
    doc["temperatura"] = t;
    doc["umidade"] = h;
    doc["sensor"] = s;


    String json;
    serializeJson(doc, json);

    int httpResponseCode = http.PUT(json);

    if (httpResponseCode > 0)
    {
      String response = http.getString();
      Serial.println("Resposta da API:");
      Serial.println(response);
    }
    else
    {
      Serial.print("Erro ao enviar dados: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi não conectado.");
  }
}

void leitura_sensores(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  s = false;

  if (isnan(h) || isnan(t))
  {
    Serial.println("Erro de leitura do DHT!");
    return;
  }
  
  Serial.print("Temperatura: ");
  Serial.println(t);

  // Controle do ar condicionado
  if (t >= 20.0)
  {
    digitalWrite(led1, LOW);
    digitalWrite(Relay, HIGH);
  }
  else
  {
    digitalWrite(led1, HIGH);
    digitalWrite(Relay, LOW);
  }

  // Sensor de movimento
  int motion = digitalRead(pirPino);
  Serial.print("Movimento: ");
  Serial.println(motion);

  if (motion == HIGH)
  {
    digitalWrite(ledPino, HIGH);
    Serial.println("Movimento detectado!");    
  }
  else
  {
    digitalWrite(ledPino, LOW);
    Serial.println("Sem movimento.");
  }
  s = motion;
  if(motion_anterior != motion)
  {
    envia_dados();
    motion_anterior = motion;
  }

}

void setup()
{
  Serial.begin(115200);

  // Inicialização do sensor DHT22
  dht.begin();

  // Configuração dos pinos
  pinMode(led1, OUTPUT);
  pinMode(Relay, OUTPUT);
  pinMode(ledPino, OUTPUT);
  pinMode(pirPino, INPUT);

  // Conectar ao WiFi
  connectToWiFi();
}

void loop()
{
  tempo = millis(); /*Atualização do valor dentro da variável*/

   if (tempo - auxiliar_sensor >= 500){
    leitura_sensores();
     auxiliar_sensor = tempo;
   }
 }


