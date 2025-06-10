#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

// Configurações do WiFi
#define WIFI_SSID "iPhone de Henrique"
#define WIFI_PASSWORD "Henrique0903"

// Configurações do Firebase
#define FIREBASE_URL "https://lixeira-inteligente-ca7a1-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBGJ6a1vWZc4kCcStSFiJvJT7F_uZbsi1M"

// Comunicação com Arduino
HardwareSerial SerialArduino(2);

// Variáveis de controle
unsigned long lastCommandCheck = 0;
unsigned long lastStatusUpdate = 0;
unsigned long lastWiFiCheck = 0;
unsigned long wifiReconnectAttempts = 0;
String lastCommand = "";
bool wifiConnected = false;

void setup() {
  Serial.begin(115200);
  delay(2000); // Aumentar delay inicial
  
  Serial.println("\n\n=== LIXEIRA INTELIGENTE ESP32 ===");
  Serial.println("Versão: 2.0 - Conectividade Melhorada");
  Serial.println("Inicializando sistema...");
  
  // Configura comunicação com Arduino
  SerialArduino.begin(9600, SERIAL_8N1, 16, 17);
  delay(1000);
  Serial.println("✅ Comunicação Arduino configurada (RX=16, TX=17)");
  
  // Teste de comunicação com Arduino
  Serial.println("🔧 Testando comunicação com Arduino...");
  SerialArduino.println("TEST");
  delay(2000);
  
  if (SerialArduino.available()) {
    String response = SerialArduino.readString();
    Serial.println("📨 Arduino respondeu: " + response);
  } else {
    Serial.println("⚠ Arduino não respondeu ao teste inicial");
  }
  
  // Configurações WiFi otimizadas
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Desabilita power saving
  WiFi.setAutoReconnect(true);
  
  // Conecta WiFi
  connectWiFi();
  
  Serial.println("=== SETUP CONCLUÍDO ===");
  Serial.println("Sistema pronto para receber comandos!");
  Serial.println("Monitorando Firebase a cada 1 segundo...");
  printSystemInfo();
}

void connectWiFi() {
  Serial.println("\n📶 Iniciando conexão WiFi...");
  Serial.println("SSID: " + String(WIFI_SSID));
  
  // Desconecta primeiro se já estava conectado
  WiFi.disconnect();
  delay(1000);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  int maxAttempts = 40; // Aumentar tentativas
  
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // A cada 10 tentativas, mostrar status
    if (attempts % 10 == 0) {
      Serial.println();
      Serial.print("Tentativa " + String(attempts) + "/" + String(maxAttempts));
      Serial.print(" - Status WiFi: ");
      printWiFiStatus();
      Serial.print("Continuando");
    }
  }
  
  Serial.println(); // Nova linha após os pontos
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    wifiReconnectAttempts = 0;
    Serial.println("✅ WiFi CONECTADO COM SUCESSO!");
    Serial.print("📍 IP Local: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Força do sinal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("🌐 Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("🔧 DNS: ");
    Serial.println(WiFi.dnsIP());
    
    // Teste de conectividade
    testInternetConnection();
    
  } else {
    wifiConnected = false;
    wifiReconnectAttempts++;
    Serial.println("❌ FALHA na conexão WiFi!");
    Serial.print("Status final: ");
    printWiFiStatus();
    Serial.println("Tentativas de reconexão: " + String(wifiReconnectAttempts));
    
    if (wifiReconnectAttempts < 3) {
      Serial.println("🔄 Tentando novamente em 5 segundos...");
      delay(5000);
      connectWiFi(); // Recursão controlada
    }
  }
}

void printWiFiStatus() {
  wl_status_t status = WiFi.status();
  switch (status) {
    case WL_IDLE_STATUS:
      Serial.println("IDLE");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("SSID NÃO ENCONTRADO");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("SCAN COMPLETO");
      break;
    case WL_CONNECTED:
      Serial.println("CONECTADO");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("FALHA NA CONEXÃO");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("CONEXÃO PERDIDA");
      break;
    case WL_DISCONNECTED:
      Serial.println("DESCONECTADO");
      break;
    default:
      Serial.println("STATUS DESCONHECIDO: " + String(status));
      break;
  }
}

void testInternetConnection() {
  Serial.println("🌐 Testando conectividade com a internet...");
  
  HTTPClient http;
  http.begin("http://httpbin.org/ip");
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("✅ Conectividade com internet OK!");
    String payload = http.getString();
    Serial.println("IP público detectado: " + payload);
  } else {
    Serial.println("⚠ Problema de conectividade com internet");
    Serial.println("Código HTTP: " + String(httpCode));
  }
  
  http.end();
}

void checkCommand() {
  if (!wifiConnected) {
    Serial.println("⚠ WiFi desconectado - pulando verificação de comando");
    return;
  }
  
  Serial.print("🔍 Verificando comando... ");
  
  HTTPClient http;
  String url = String(FIREBASE_URL) + "/lixeira/comando.json";
  http.begin(url);
  http.setTimeout(8000); // Aumentar timeout
  http.addHeader("User-Agent", "ESP32-LixeiraInteligente/1.0");
  
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    payload.trim();
    
    Serial.print("Resposta: ");
    Serial.println(payload);
    
    // Remove aspas se existirem
    if (payload.startsWith("\"") && payload.endsWith("\"")) {
      payload = payload.substring(1, payload.length() - 1);
    }
    
    if (payload.length() > 0 && payload != "null" && payload != lastCommand) {
      Serial.println("🎯 NOVO COMANDO DETECTADO!");
      Serial.println("Anterior: [" + lastCommand + "] -> Novo: [" + payload + "]");
      lastCommand = payload;
      
      if (payload == "abrir") {
        Serial.println("🔓 EXECUTANDO: Abrir tampa");
        
        // Enviar comando com confirmação
        SerialArduino.write('O');
        SerialArduino.flush(); // Garantir que foi enviado
        Serial.println("📤 Comando 'O' enviado para Arduino");
        
        // Aguardar resposta do Arduino
        Serial.println("⏳ Aguardando confirmação do Arduino...");
        unsigned long timeout = millis() + 3000; // 3 segundos timeout
        bool responseReceived = false;
        
        while (millis() < timeout && !responseReceived) {
          if (SerialArduino.available()) {
            String response = SerialArduino.readStringUntil('\n');
            response.trim();
            if (response.length() > 0) {
              Serial.println("📨 Arduino respondeu: " + response);
              responseReceived = true;
            }
          }
          delay(10);
        }
        
        if (!responseReceived) {
          Serial.println("⚠ Arduino não respondeu - possível problema de comunicação");
        }
        
        updateStatus("aberta");
      }
      else if (payload == "fechar") {
        Serial.println("🔒 EXECUTANDO: Fechar tampa");
        
        // Enviar comando com confirmação
        SerialArduino.write('C');
        SerialArduino.flush(); // Garantir que foi enviado
        Serial.println("📤 Comando 'C' enviado para Arduino");
        
        // Aguardar resposta do Arduino
        Serial.println("⏳ Aguardando confirmação do Arduino...");
        unsigned long timeout = millis() + 3000; // 3 segundos timeout
        bool responseReceived = false;
        
        while (millis() < timeout && !responseReceived) {
          if (SerialArduino.available()) {
            String response = SerialArduino.readStringUntil('\n');
            response.trim();
            if (response.length() > 0) {
              Serial.println("📨 Arduino respondeu: " + response);
              responseReceived = true;
            }
          }
          delay(10);
        }
        
        if (!responseReceived) {
          Serial.println("⚠ Arduino não respondeu - possível problema de comunicação");
        }
        
        updateStatus("fechada");
      }
      else {
        Serial.println("⚠ Comando não reconhecido: [" + payload + "]");
      }
    } else {
      Serial.println("OK (sem mudanças)");
    }
  } else {
    Serial.print("❌ Erro HTTP: ");
    Serial.println(httpCode);
    
    // Diagnóstico detalhado de erros
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println("Erro: Conexão recusada");
    } else if (httpCode == HTTPC_ERROR_SEND_HEADER_FAILED) {
      Serial.println("Erro: Falha ao enviar cabeçalho");
    } else if (httpCode == HTTPC_ERROR_SEND_PAYLOAD_FAILED) {
      Serial.println("Erro: Falha ao enviar dados");
    } else if (httpCode == HTTPC_ERROR_NOT_CONNECTED) {
      Serial.println("Erro: Não conectado");
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println("Erro: Conexão perdida");
    } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
      Serial.println("Erro: Timeout de leitura");
    }
    
    // Se erro de conexão, verificar WiFi
    if (httpCode < 0) {
      Serial.println("Verificando status do WiFi...");
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi desconectado detectado!");
        wifiConnected = false;
      }
    }
  }
  
  http.end();
}

void updateStatus(String status) {
  if (!wifiConnected) {
    Serial.println("⚠ WiFi desconectado - não foi possível atualizar status");
    return;
  }
  
  Serial.print("📝 Atualizando status para: ");
  Serial.println(status);
  
  HTTPClient http;
  String url = String(FIREBASE_URL) + "/lixeira/status.json";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-LixeiraInteligente/1.0");
  http.setTimeout(8000);
  
  String jsonData = "\"" + status + "\"";
  int httpCode = http.PUT(jsonData);
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("✅ Status atualizado com sucesso!");
  } else {
    Serial.print("❌ Erro ao atualizar status: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

void sendSensorData(String data) {
  if (!wifiConnected) return;
  
  Serial.println("📊 Enviando dados do sensor: " + data);
  
  HTTPClient http;
  String url = String(FIREBASE_URL) + "/lixeira/sensores.json";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-LixeiraInteligente/1.0");
  http.setTimeout(8000);
  
  String jsonData = "\"" + data + "\"";
  int httpCode = http.PUT(jsonData);
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("✅ Dados do sensor enviados!");
  } else {
    Serial.println("❌ Erro ao enviar dados do sensor: " + String(httpCode));
  }
  
  http.end();
}

void sendAlert(String alert) {
  if (!wifiConnected) return;
  
  Serial.println("🚨 Enviando alerta: " + alert);
  
  HTTPClient http;
  String url = String(FIREBASE_URL) + "/lixeira/sensores.json";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32-LixeiraInteligente/1.0");
  http.setTimeout(8000);
  
  String jsonData = "\"" + alert + "\"";
  int httpCode = http.PUT(jsonData);
  
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("✅ Alerta enviado!");
  } else {
    Serial.println("❌ Erro ao enviar alerta: " + String(httpCode));
  }
  
  http.end();
}

void printSystemInfo() {
  Serial.println("\n=== INFORMAÇÕES DO SISTEMA ===");
  Serial.println("Chip ID: " + String((uint32_t)ESP.getEfuseMac(), HEX));
  Serial.println("CPU Freq: " + String(ESP.getCpuFreqMHz()) + " MHz");
  Serial.println("Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB");
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Arduino Core: " + String(ESP.getSdkVersion()));
  Serial.println("===============================\n");
}

void printStatus() {
  Serial.println("\n=== STATUS DO SISTEMA ===");
  Serial.print("WiFi: ");
  Serial.print(wifiConnected ? "✅ Conectado" : "❌ Desconectado");
  Serial.print(" (");
  printWiFiStatus();
  Serial.print(")");
  Serial.println();
  
  if (wifiConnected) {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Sinal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  
  Serial.print("Último comando: ");
  Serial.println(lastCommand.length() > 0 ? lastCommand : "nenhum");
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" segundos");
  Serial.print("Memória livre: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("Tentativas reconexão: ");
  Serial.println(wifiReconnectAttempts);
  Serial.println("========================\n");
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      Serial.println("🔄 WiFi desconectou! Tentando reconectar...");
      wifiConnected = false;
    }
    connectWiFi();
  } else if (!wifiConnected) {
    Serial.println("🔄 WiFi reconectado!");
    wifiConnected = true;
    wifiReconnectAttempts = 0;
  }
}

void loop() {
  // Verificar conexão WiFi a cada 10 segundos
  if (millis() - lastWiFiCheck >= 10000) {
    checkWiFiConnection();
    lastWiFiCheck = millis();
  }
  
  // Verificar comandos a cada 1 segundo (mais estável que 500ms)
  if (millis() - lastCommandCheck >= 1000) {
    checkCommand();
    lastCommandCheck = millis();
  }
  
  // Status do sistema a cada 30 segundos
  if (millis() - lastStatusUpdate >= 30000) {
    printStatus();
    lastStatusUpdate = millis();
  }
  
  // Processar dados do Arduino
  if (SerialArduino.available()) {
    String dados = SerialArduino.readStringUntil('\n');
    dados.trim();
    
    if (dados.length() > 0) {
      Serial.println("🤖 Arduino: " + dados);
      
      // Processar diferentes tipos de mensagens
      if (dados.startsWith("STATUS:")) {
        String status = dados.substring(7);
        Serial.println("📱 Status do Arduino: " + status);
        updateStatus(status);
      }
      else if (dados.startsWith("SENSOR:")) {
        String sensorData = dados.substring(7);
        Serial.println("📊 Sensor: " + sensorData);
        sendSensorData(sensorData);
      }
      else if (dados.startsWith("ALERT:")) {
        String alert = dados.substring(6);
        Serial.println("🚨 Alerta: " + alert);
        sendAlert(alert);
      }
      else {
        Serial.println("📝 Mensagem: " + dados);
      }
    }
  }
  
  delay(100); // Delay para estabilidade
}