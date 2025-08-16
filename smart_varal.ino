#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================= DEFINIÇÕES =================
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_CONTROL = "varal/controle";

// Pinos do motor
#define IN1 4
#define IN2 17
#define IN3 18
#define IN4 21
#define SENSOR_GPIO 5   // Pino do sensor de umidade

// Configurações do motor
#define WET_ACTIVE_LEVEL 0
#define STEP_DELAY_MS 12
#define SPIN_MS 45000
#define DIR_WET (+1)
#define DIR_DRY (-1)

// Credenciais WiFi
const char* WIFI_SSID = "MarceloCelular";
const char* WIFI_PASS = "marcelo2";

// Sequência de passos do motor
const uint8_t fullstep_seq[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}
};

// Ordem das bobinas
int ORDER[4] = {0, 1, 2, 3};

// ================= VARIÁVEIS GLOBAIS =================
volatile bool g_is_wet = false;          // false = seco, true = molhado
volatile bool g_motor_running = false;
int s_idx = 0;
bool modo_manual = false;
bool status_varal = true;                // true = liberado (estado inicial desejado)

// Objetos de conexão
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Controle de conexão
bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 10000; // 10 segundos

// ================= FUNÇÕES DO MOTOR =================
void set_raw(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    digitalWrite(IN1, a);
    digitalWrite(IN2, b);
    digitalWrite(IN3, c);
    digitalWrite(IN4, d);
}

void coils_off() { 
    set_raw(0, 0, 0, 0); 
}

void set_ordered(const uint8_t s[4]) {
    uint8_t out[4] = {0, 0, 0, 0};
    out[ORDER[0]] = s[0];
    out[ORDER[1]] = s[1];
    out[ORDER[2]] = s[2];
    out[ORDER[3]] = s[3];
    set_raw(out[0], out[1], out[2], out[3]);
}

void step_tick_full(int dir) {
    s_idx = (s_idx + (dir > 0 ? 1 : 3)) % 4;
    set_ordered(fullstep_seq[s_idx]);
    delay(STEP_DELAY_MS);
}

void motor_spin_ms(int ms, int dir) {
    g_motor_running = true;
    Serial.println("[MOTOR] Iniciando movimento");
    
    unsigned long startTime = millis();
    while (millis() - startTime < ms) {
        step_tick_full(dir);
    }
    
    coils_off();
    g_motor_running = false;
    Serial.println("[MOTOR] Movimento concluído");
}

// ================= INTERRUPÇÃO DO SENSOR =================
void IRAM_ATTR sensor_isr() {
    g_is_wet = (digitalRead(SENSOR_GPIO) == WET_ACTIVE_LEVEL);
}

// ================= CONFIGURAÇÃO INICIAL =================
void configure_motor_pins() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    coils_off();
}

void configure_sensor() {
    pinMode(SENSOR_GPIO, INPUT);
    g_is_wet = (digitalRead(SENSOR_GPIO) == WET_ACTIVE_LEVEL);
    attachInterrupt(digitalPinToInterrupt(SENSOR_GPIO), sensor_isr, CHANGE);
}

// ================= MQTT =================
void publishFullStatus() {
    if (mqttClient.connected()) {
        DynamicJsonDocument doc(256);
        doc["estado"] = status_varal ? "liberado" : "recolhido";
        doc["modo"] = modo_manual ? "manual" : "auto";
        doc["umidade"] = g_is_wet ? "molhado" : "seco";
        
        String output;
        serializeJson(doc, output);
        
        mqttClient.publish(MQTT_TOPIC_CONTROL, output.c_str());
        Serial.println("[STATUS] " + output);
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.printf("[MQTT] Mensagem recebida [%s]: %s\n", topic, message);
    
    if (strcmp(topic, MQTT_TOPIC_CONTROL) == 0) {
        if (strcmp(message, "manual") == 0) {
            modo_manual = true;
            Serial.println("[MODO] Manual ativado");
            publishFullStatus();
        } 
        else if (strcmp(message, "auto") == 0) {
            modo_manual = false;
            Serial.println("[MODO] Automático ativado");
            publishFullStatus();
        } 
        else if (strcmp(message, "recolher") == 0) {
            if (!status_varal) {
                Serial.println("[MOTOR] Ignorado: varal já está recolhido");
            } else {
                motor_spin_ms(SPIN_MS, DIR_DRY);
                status_varal = false; // agora recolhido
            }
            publishFullStatus();
        } 
        else if (strcmp(message, "liberar") == 0) {
            if (status_varal) {
                Serial.println("[MOTOR] Ignorado: varal já está liberado");
            } else {
                motor_spin_ms(SPIN_MS, DIR_WET);
                status_varal = true; // agora liberado
            }
            publishFullStatus();
        } 
        else if (strcmp(message, "consulta") == 0) { 
            publishFullStatus();
        }
    }
}

bool connectMQTT() {
    Serial.println("[MQTT] Tentando conexão...");
    
    String clientId = "ESP32Varal-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("[MQTT] Conectado com sucesso");
        mqttClient.subscribe(MQTT_TOPIC_CONTROL);
        publishFullStatus();
        return true;
    } else {
        Serial.printf("[MQTT] Falha na conexão (rc=%d)\n", mqttClient.state());
        return false;
    }
}

// ================= WIFI =================
void checkWiFiConnection() {
    static unsigned long lastCheck = 0;
    const unsigned long checkInterval = 5000;
    
    if (millis() - lastCheck > checkInterval) {
        lastCheck = millis();
        
        bool currentStatus = (WiFi.status() == WL_CONNECTED);
        
        if (currentStatus != wifiConnected) {
            wifiConnected = currentStatus;
            if (wifiConnected) {
                Serial.printf("[WiFi] Conectado\nIP: %s\n", WiFi.localIP().toString().c_str());
            } else {
                Serial.println("[WiFi] Desconectado");
            }
        }
    }
}

// ================= CONTROLE PRINCIPAL =================
void ctrl_task() {
    static bool last_state = g_is_wet;  // estado anterior do sensor
    static bool first_run = true;
    
    if (first_run) {
        Serial.printf("[SISTEMA] Inicial: %s, varal %s\n",
                      g_is_wet ? "MOLHADO" : "SECO",
                      status_varal ? "LIBERADO" : "RECOLHIDO");
        first_run = false;
        publishFullStatus();
    }

    if (!g_motor_running && (g_is_wet != last_state) && !modo_manual) {
        bool became_wet = g_is_wet;   // true se mudou para MOLHADO
        last_state = g_is_wet;

        if (became_wet) {
            // SECO → MOLHADO  => RECOLHER se estiver LIBERADO
            Serial.println("[AÇÃO] SECO → MOLHADO (automático)");
            if (status_varal) { // estava liberado
                motor_spin_ms(SPIN_MS, DIR_DRY); // recolher
                status_varal = false;
            }
        } else {
            // MOLHADO → SECO  => LIBERAR se estiver RECOLHIDO
            Serial.println("[AÇÃO] MOLHADO → SECO (automático)");
            if (!status_varal) { // estava recolhido
                motor_spin_ms(SPIN_MS, DIR_WET); // liberar
                status_varal = true;
            }
        }
        publishFullStatus();
    }
}

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    while(!Serial);
    delay(1000);
    
    Serial.println("\n=== SISTEMA VARAL INTELIGENTE ===");
    Serial.println("Inicializando hardware...");
    
    configure_motor_pins();
    configure_sensor();
    
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
    mqttClient.setKeepAlive(60);
    mqttClient.setSocketTimeout(30);
    mqttClient.setBufferSize(1024);
}

// ================= LOOP PRINCIPAL =================
void loop() {
    checkWiFiConnection();
    
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL) {
                lastReconnectAttempt = millis();
                if (connectMQTT()) {
                    mqttConnected = true;
                } else {
                    mqttConnected = false;
                }
            }
        } else {
            mqttClient.loop();
        }
    }
    
    ctrl_task();
    delay(50);
}
