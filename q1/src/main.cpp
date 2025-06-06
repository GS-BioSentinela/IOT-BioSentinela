/********************************************************************
 * Projeto: Envio de Dados MQTT com ESP32 - Global Solution 2025
 * Autor: André Tritiack
 * Placa: DOIT ESP32 DEVKIT V1
 * 
 * Descrição:
 * Este projeto conecta o ESP32 a uma rede Wi-Fi e a um Broker MQTT.
 * A cada 10 segundos, envia uma mensagem JSON contendo:
 * - ID do grupo
 * - ID do módulo
 * - IP local
 * - Dados de temperatura e umidade do sensor DHT22
 * - Valor analógico do potenciômetro (0-4095)
 * 
 * Baseado no repositório original:
 * https://github.com/arnaldojr/iot-esp32-wokwi-vscode
 * Professor Arnaldo Viana - FIAP
 ********************************************************************/

//----------------------------------------------------------
// Bibliotecas já disponíveis no ambiente ESP32

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//----------------------------------------------------------
// Definições e configurações

#define boardLED 2      // LED onboard
#define DHTPIN 12       // Pino de dados do DHT
#define DHTTYPE DHT22   // DHT22 (AM2302)
#define MQ2PIN 34       // Simulação do sensor de gás (ADC)
#define PIRPIN 13       // Simulação do sensor de movimento (digital)

// Identificadores
const char* ID        = "BioSentinela_Grupo4";
const char* moduleID  = "BioESP32_Node1";

// Wi-Fi (NÃO ALTERAR)
const char* SSID      = "Wokwi-GUEST";
const char* PASSWORD  = "";

// MQTT Broker (NÃO ALTERAR)
const char* BROKER_MQTT  = "172.208.54.189";
const int   BROKER_PORT  = 1883;
const char* mqttUser     = "gs2025";
const char* mqttPassword = "q1w2e3r4";

// Novo tópico MQTT
#define TOPICO_PUBLISH  "biosentinela/esp32/monitoramento"

//----------------------------------------------------------
// Variáveis globais

WiFiClient espClient;
PubSubClient MQTT(espClient);
JsonDocument doc;
char buffer[256];
DHT dht(DHTPIN, DHTTYPE);

float temperatura;
float umidade;
int mq2Value;
bool movimentoDetectado;

//----------------------------------------------------------
// Conexão Wi-Fi

void initWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Conectando ao Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void reconectaWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconectando Wi-Fi...");
        initWiFi();
    }
}

//----------------------------------------------------------
// Conexão MQTT

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    while (!MQTT.connected()) {
        Serial.println("Conectando ao Broker MQTT...");
        if (MQTT.connect(moduleID, mqttUser, mqttPassword)) {
            Serial.println("Conectado ao Broker!");
        } else {
            Serial.print("Falha na conexão. Estado: ");
            Serial.println(MQTT.state());
            delay(2000);
        }
    }
}

void verificaConexoesWiFiEMQTT() {
    reconectaWiFi();
    if (!MQTT.connected()) {
        initMQTT();
    }
    MQTT.loop();
}

//----------------------------------------------------------
// Envio MQTT

void enviaEstadoOutputMQTT() {
    MQTT.publish(TOPICO_PUBLISH, buffer);
    Serial.println("Mensagem publicada com sucesso!");
}

void piscaLed() {
    digitalWrite(boardLED, HIGH);
    delay(300);
    digitalWrite(boardLED, LOW);
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
    initMQTT();
}

//----------------------------------------------------------
// Loop principal

void loop() {
    verificaConexoesWiFiEMQTT();

    temperatura = dht.readTemperature();
    umidade = dht.readHumidity();
    mq2Value = analogRead(MQ2PIN);
    movimentoDetectado = digitalRead(PIRPIN);

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
        doc["Temperatura"] = "Erro na leitura";
        doc["Umidade"] = "Erro na leitura";
    }

    doc["GasFumaca"] = mq2Value;
    doc["Movimento"] = movimentoDetectado ? "Detectado" : "Ausente";

    // Lógica de alerta
    bool alerta = (temperatura > 40.0 || mq2Value > 600 || movimentoDetectado);
    doc["Alerta"] = alerta ? "ATIVO" : "Normal";

    // Serializa
    serializeJson(doc, buffer);

    // Exibe no serial e envia
    Serial.println(buffer);
    enviaEstadoOutputMQTT();
    piscaLed();

    delay(10000);  // 10 segundos entre envios
}
