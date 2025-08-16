# Varal Inteligente com ESP32

![ESP32](https://img.shields.io/badge/ESP32-Development_Platform-blue) ![Arduino](https://img.shields.io/badge/Arduino_IDE-Compatible-green) ![MQTT](https://img.shields.io/badge/Protocol-MQTT-orange) ![Telegram](https://img.shields.io/badge/Integration-Telegram_Bot-blue)

Um sistema automatizado para controle de varal baseado em umidade e comandos remotos via Telegram.

## 📋 Pré-requisitos

- Placa ESP32
- IDE Arduino instalada
- Conexão Wi-Fi estável
- Conta no Telegram (para controle via bot)

## 🛠 Instalação

### 1. Instalar a IDE Arduino
Baixe e instale a IDE Arduino do [site oficial](https://www.arduino.cc/en/software).

### 2. Configurar Suporte ao ESP32
1. Abra a IDE Arduino
2. Vá em `Arquivo > Preferências`
3. Adicione esta URL em "URLs adicionais para gerenciadores de placas": https://dl.espressif.com/dl/package_esp32_index.json
4. Vá para `Ferramentas > Placa > Gerenciador de Placas`
5. Busque por `ESP32` e instale

### 3. Instalar Bibliotecas Necessárias
No Gerenciador de Bibliotecas (`Sketch > Incluir Biblioteca > Gerenciar Bibliotecas`), instale:
- `PubSubClient` (para MQTT)
- `ArduinoJson` (para manipulação de JSON)
- `WiFi` (já instalada com o ESP32)

## ⚙ Configuração

### Conexões do Hardware
| Componente    | Pino ESP32 |
|--------------|-----------|
| Motor IN1    | GPIO 4    |
| Motor IN2    | GPIO 17   |
| Motor IN3    | GPIO 18   |
| Motor IN4    | GPIO 21   |
| Sensor Umidade | GPIO 5   |

### Configuração do Software
No arquivo `varal_inteligente.ino`, altere as seguintes variáveis:

// Configurações de Rede
const char* WIFI_SSID = "Seu_SSID";
const char* WIFI_PASS = "Sua_Senha";

// Configurações MQTT
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_CONTROL = "varal/controle";
const char* MQTT_TOPIC_STATUS = "varal/estado";

## 📤 Upload do Código

1. Conecte o ESP32 via USB
2. Selecione:
   - **Placa:** `ESP32 Dev Module`
   - **Porta COM** correspondente
3. Clique no botão de Upload (→)

## 🤖 Configuração do Telegram

1. Crie um bot conversando com [@BotFather](https://t.me/BotFather)
2. Guarde o token fornecido
3. Insira o token no código (se necessário)

## 🚀 Funcionamento

**Funcionalidades:**
- ✅ Controle automático baseado na umidade
- 📲 Comandos manuais via Telegram:

| Comando    | Ação                          |
|------------|-------------------------------|
| `/recolher`| Recolhe o varal               |
| `/liberar` | Libera o varal                |
| `/status`  | Verifica o estado atual       |

## 📊 Monitoramento

**Monitor Serial** (`Ferramentas > Monitor Serial`):
- 📝 Ver logs do sistema
- 🐞 Depurar problemas
- ⏱ Monitorar status em tempo real

## 🔧 Troubleshooting

| Problema                     | Solução                          |
|------------------------------|-----------------------------------|
| Falha na conexão Wi-Fi       | Verifique SSID e senha           |
| Problemas com MQTT           | Teste com broker público alternativo |
| Motor não responde           | Confira conexões dos pinos       |
