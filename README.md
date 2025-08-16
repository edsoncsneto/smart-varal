1. Instalar a IDE Arduino

Baixe e instale a IDE Arduino.

Após a instalação, abra a IDE Arduino.

2. Instalar Suporte ao ESP32 na IDE Arduino

Vá em Arquivo > Preferências na IDE.

Na seção URLs adicionais para gerenciadores de placas, adicione o seguinte link:

https://dl.espressif.com/dl/package_esp32_index.json


Vá para Ferramentas > Placa > Gerenciador de Placas, busque por ESP32 e clique em Instalar.

3. Instalar as Bibliotecas Necessárias

Na IDE Arduino, vá para Sketch > Incluir Biblioteca > Gerenciar Bibliotecas....

Busque e instale as seguintes bibliotecas:

WiFi (instalada por padrão com o ESP32)

PubSubClient (para MQTT)

ArduinoJson (para manipulação de JSON)

4. Configuração do Código

Conecte o ESP32 ao computador via cabo USB.

Abra o código varal_inteligente.ino na IDE Arduino.

Configuração do Wi-Fi e MQTT:

No código, localize as seguintes variáveis e substitua pelos seus dados:

const char* WIFI_SSID = "Seu_SSID";
const char* WIFI_PASS = "Sua_Senha";
const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC_CONTROL = "varal/controle";
const char* MQTT_TOPIC_STATUS = "varal/estado";


Altere o SSID e senha da sua rede Wi-Fi.

O broker MQTT está configurado para usar o broker público HiveMQ, mas você pode mudar para um broker privado se preferir.

5. Configuração do Motor e Sensor

Pinos do Motor:

IN1 (GPIO 4), IN2 (GPIO 17), IN3 (GPIO 18) e IN4 (GPIO 21) controlam o motor de passo.

Certifique-se de que o motor e o driver estão conectados corretamente aos pinos do ESP32.

Sensor de Umidade:

O sensor de umidade está conectado ao pino GPIO 5. Se você usar um sensor diferente, ajuste o pino conforme necessário.

6. Carregar o Código no ESP32

Selecione a placa ESP32 em Ferramentas > Placa > ESP32 Dev Module.

Selecione a porta COM do ESP32 em Ferramentas > Porta.

Clique em Carregar (ícone de seta) para enviar o código para o ESP32.

7. Configuração do Bot no Telegram

Crie um bot no Telegram conversando com o BotFather no Telegram.

Guarde o token do bot fornecido pelo BotFather.

No seu código, insira o token do bot em uma variável, caso precise de alguma configuração adicional para comunicação.

8. Executando o Sistema

Após carregar o código no ESP32, o sistema se conectará à rede Wi-Fi e ao broker MQTT.

O varal será controlado automaticamente com base na umidade ou manualmente via comandos do Telegram.

Comandos como "recolher" ou "liberar" podem ser enviados através do bot no Telegram para controlar o varal.

9. Testes e Monitoramento

Utilize o monitor serial da IDE Arduino para ver os logs do ESP32 e depurar qualquer problema.

O sistema irá publicar o status em tempo real no tópico MQTT configurado, e o bot no Telegram exibirá o status para o usuário.
