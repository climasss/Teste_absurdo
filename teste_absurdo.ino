#include <HardwareSerial.h>

// --- Definições de Pinos ---
// LoRa (Envio)
#define PIN_LORA_RX 16
#define PIN_LORA_TX 17
#define PIN_GND_LORA 18 // Pino 18 atuará como GND para o módulo LoRa

// STM (Recebimento)
#define PIN_STM_RX 19
#define PIN_STM_TX 21

// --- Instância das Seriais ---
// UART2 para o LoRa (Padrão no ESP32 costuma ser 16/17)
HardwareSerial SerialLoRa(2); 
// UART1 para o STM (Vamos remapear para 19/21)
HardwareSerial SerialSTM(1);

// --- Estrutura de Dados (Vinda do STM) ---
struct Dados {
  int buffer_star1;
  int buffer_star2;
  float latitude;
  float longitude;
  int temperatura_motor;
  int temperatura_cvt;
  int velocidade;
  int odometro;
  int hora;
  int minuto;
  int mes;
  int ano;
  int altitude;
  int rpm_motor;
  bool batteryLevel;
  bool farol;
  bool conct_LAN;
  int low_gas;
  bool high_gas;
  int buffer_end1;
  int buffer_end2;
};

Dados buffer; // Armazena os últimos dados recebidos
unsigned long previousMillis = 0;

void setup() {
  // 1. Inicializa Serial USB (Debug)
  Serial.begin(9600);
  Serial.println("Iniciando ESP32 Gateway...");

  // 2. Configura Pino GND Virtual (apenas o 18, pois o 19 é RX do STM)
  pinMode(PIN_GND_LORA, OUTPUT);
  digitalWrite(PIN_GND_LORA, LOW);

  // 3. Inicializa Serial do STM (Recebe dados a 115200)
  // SerialSTM.begin(baud, config, rx, tx);
  SerialSTM.begin(115200, SERIAL_8N1, PIN_STM_RX, PIN_STM_TX);

  // 4. Inicializa Serial do LoRa (Envia dados a 9600)
  SerialLoRa.begin(9600, SERIAL_8N1, PIN_LORA_RX, PIN_LORA_TX);
}

void loop() {
  // ---------------------------------------------------------
  // 1. RECEBIMENTO DE DADOS DO STM (Leitura constante)
  // ---------------------------------------------------------
  if (SerialSTM.available()) {
    String mensagem = SerialSTM.readStringUntil('\n'); // Lê até pular linha (ajuste conforme envio do STM)
    mensagem.trim(); // Limpa espaços
  
    if (mensagem.length() > 0) {
      // Parseia os dados (Quebra a string por vírgulas)
      int startIndex = 0;
      int endIndex;
      String valores[23]; // Array para guardar as partes temporariamente

      // Preenche o array de strings
      for (int i = 0; i < 23; i++) {
        endIndex = mensagem.indexOf(',', startIndex);
        if (endIndex == -1) endIndex = mensagem.length();
        valores[i] = mensagem.substring(startIndex, endIndex);
        startIndex = endIndex + 1;
      }

      // Atualiza a Struct global com os dados novos
      // É importante verificar se o array foi preenchido para evitar erros de conversão
      if (valores[0].length() > 0) {
          buffer.buffer_star1 = valores[0].toInt();
          // ... (resto dos campos ignorados na struct se não forem usados no envio) ...
          buffer.temperatura_motor = valores[4].toInt();
          buffer.temperatura_cvt = valores[5].toInt();
          buffer.velocidade = valores[6].toInt();
          buffer.odometro = valores[7].toInt();
          buffer.rpm_motor = valores[13].toInt();
          buffer.batteryLevel = valores[14].toInt() == 1;
          buffer.farol = valores[15].toInt() == 1;
          buffer.low_gas = valores[17].toInt();
          buffer.high_gas = valores[18].toInt() == 1;
          
          // Debug opcional no USB
          // Serial.println("Dados atualizados do STM!");
      }
    }
  }

      String dataToSend = "CAPI, " + 
                        String(buffer.velocidade) + ", " + 
                        String(buffer.velocidade) + ", " +
                        String(buffer.temperatura_motor) + ", " + 
                        String(buffer.temperatura_cvt) + ", " +
                        String(buffer.odometro) + ", " + 
                        String(buffer.farol) + ", " + 
                        String(buffer.velocidade);

  // ---------------------------------------------------------
  // 2. ENVIO PARA O LORA (Temporizado)
  // ---------------------------------------------------------
  if (millis() - previousMillis >= 1100) {
    previousMillis = millis();
    SerialLoRa.print(dataToSend);
    
    // Debug no PC para confirmar saída
    Serial.print("Enviado LoRa: ");
    Serial.println(dataToSend);
  }
}
