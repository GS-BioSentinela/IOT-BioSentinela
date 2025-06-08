/********************************************************************
 * Projeto: Envio de Dados HTTP com ESP32 - BioSentinela
 * Autor: Juliana + Adaptado por ChatGPT
 * Placa: DOIT ESP32 DEVKIT V1
 * Versão: 2.0 - Otimizado para VM
 ********************************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//----------------------------------------------------------
// Definições e configurações

#define boardLED 2      // LED onboard
#define DHTPIN 12       // Pino de dados do DHT
#define DHTTYPE DHT22   // Tipo do sensor
#define MQ2PIN 34       // Pino do MQ2 (gás)
#define PIRPIN 13       // Pino do PIR (movimento)

// Identificadores do projeto
const char* ID        = "BioSentinela_Grupo4";
const char* moduleID  = "BioESP32_Node1";

const char* SSID      = "Wokwi-GUEST";
const char* PASSWORD  = "";

const char* serverUrl = "http://172.208.54.189:1880/biosentinela";  

//----------------------------------------------------------
// Variáveis globais

DHT dht(DHTPIN, DHTTYPE);
float temperatura;
float umidade;
int mq2Value;
bool movimentoDetectado;
DynamicJsonDocument doc(256);

//----------------------------------------------------------
// Conexão Wi-Fi 

void initWiFi() {
    Serial.println();
    Serial.print("Conectando ao WiFi: ");
    Serial.println(SSID);

    WiFi.begin(SSID, PASSWORD);
    
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 15) {
        delay(1000);
        Serial.print(".");
        tentativas++;
        digitalWrite(boardLED, !digitalRead(boardLED)); // Pisca LED durante conexão
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC: ");
        Serial.println(WiFi.macAddress());
        digitalWrite(boardLED, HIGH);
    } else {
        Serial.println("\nFalha na conexão WiFi!");
        digitalWrite(boardLED, LOW);
    }
}

//----------------------------------------------------------
// Envio HTTP 

void enviaDadosViaHTTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi desconectado. Tentando reconectar...");
        initWiFi();
        return;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);  
    // Prepara JSON
    doc.clear();
    doc["ID"] = ID;
    doc["Sensor"] = moduleID;
    doc["IP"] = WiFi.localIP().toString();
    doc["MAC"] = WiFi.macAddress();

    if (!isnan(temperatura) && !isnan(umidade)) {
        doc["Temperatura"] = temperatura;
        doc["Umidade"] = umidade;
    } else {
        doc["Temperatura"] = "Erro";
        doc["Umidade"] = "Erro";
    }

    doc["GasFumaca"] = mq2Value;
    doc["Movimento"] = movimentoDetectado ? "Detectado" : "Ausente";

    bool alerta = (temperatura > 40.0 || mq2Value > 600 || movimentoDetectado);
    doc["Alerta"] = alerta ? "ATIVO" : "Normal";

    String jsonString;
    serializeJson(doc, jsonString);

    // Debug no Serial
    Serial.println("\nEnviando dados para:");
    Serial.println(serverUrl);
    Serial.println("Conteúdo JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();

    // Envia requisição
    int httpCode = http.POST(jsonString);

    // Processa resposta
    if (httpCode > 0) {
        Serial.printf("Resposta HTTP: %d\n", httpCode);
        
        if (httpCode == HTTP_CODE_OK) {
            String resposta = http.getString();
            Serial.println("Resposta do servidor:");
            Serial.println(resposta);
        }
    } else {
        Serial.printf("Falha no HTTP: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    
    // Feedback visual
    for (int i = 0; i < 3; i++) {
        digitalWrite(boardLED, HIGH);
        delay(100);
        digitalWrite(boardLED, LOW);
        delay(100);
    }
}

//----------------------------------------------------------
// Setup

void setup() {
    Serial.begin(115200);
    pinMode(boardLED, OUTPUT);
    pinMode(MQ2PIN, INPUT);
    pinMode(PIRPIN, INPUT);
    digitalWrite(boardLED, LOW);

    dht.begin();
    initWiFi();

    Serial.println("\nSistema BioSentinela Iniciado");
    Serial.println("----------------------------");
}

//----------------------------------------------------------
// Loop principal com temporização precisa

void loop() {
    static unsigned long ultimoEnvio = 0;
    const unsigned long intervalo = 10000; // 10 segundos

    // Coleta de dados
    temperatura = dht.readTemperature();
    umidade = dht.readHumidity();
    mq2Value = analogRead(MQ2PIN);
    movimentoDetectado = digitalRead(PIRPIN);

    // Verifica se é hora de enviar
    if (millis() - ultimoEnvio >= intervalo) {
        enviaDadosViaHTTP();
        ultimoEnvio = millis();
    }

    // Pequena pausa para estabilidade
    delay(100);
}