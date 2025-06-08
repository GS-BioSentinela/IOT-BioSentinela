# BioSentinela  
BioSentinela é um projeto de monitoramento ambiental inteligente desenvolvido para proteger habitats silvestres, utilizando um microcontrolador ESP32 e sensores simulados no Wokwi. Os dados são enviados via protocolo HTTP para um servidor local que exibe as informações em tempo real em um dashboard web.

## Tecnologias Utilizadas  
- ESP32 (simulado)  
- Sensor de temperatura e umidade (DHT22)  
- Sensor de gás/fumaça (analógico)  
- Sensor de movimento (PIR)  
 
## Formato dos Dados (JSON)  
O ESP32 envia periodicamente um JSON via HTTP POST com os seguintes dados:  
{  
  "ID": "BioSentinela_Grupo4",  
  "Sensor": "BioESP32_Node1",  
  "IP": "10.13.37.2",  
  "MAC": "24:0A:C4:00:01:10",  
  "Temperatura": 24,  
  "Umidade": 40,  
  "GasFumaca": 0,  
  "Movimento": "Ausente",  
  "Alerta": "Normal"  
}  
## Como Executar  
1. Clone o repositório e acesse a pasta:  
   git clone https://github.com/GS-BioSentinela/IOT-BioSentinela.git 
2. Instale as dependências:  
   npm install  
3. Inicie o servidor:  
   node server.js  
4. Acesse o dashboard em:  
   http://http://172.208.54.189:1880/:3000  
 
## Funcionalidades do Dashboard  
- Exibição de temperatura e umidade com ponteiros  
- Monitoramento de gás/fumaça com indicador numérico  
- Detecção de movimento e alerta visual  
- Atualização automática a cada novo dado recebido  

## Observações  
- O projeto é acadêmico e simulado com Wokwi e PlatformIO 
- A comunicação é feita via HTTP e não utiliza MQTT  
- O dashboard funciona localmente e pode ser acessado por qualquer dispositivo na mesma rede  

Desenvolvido por BioSentinela — FIAP 2025
