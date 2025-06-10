#include <Servo.h>

Servo servo;
const int servoPin = 7;
const int openAngle = 90;
const int closeAngle = 0;

// Sensor de Tampa (abertura)
const int trigPin1 = 9;
const int echoPin1 = 8;

// Sensor de Nível (lixeira cheia)
const int trigPin2 = 4;
const int echoPin2 = 3; 

const int ledPin = 13; // LED indicador de lixeira cheia

// Variáveis de controle
bool lidOpen = false;
unsigned long lastAutoCheck = 0;
unsigned long lastSensorRead = 0;
unsigned long lastStatusSend = 0;
bool manualMode = false;
unsigned long manualModeTimer = 0;

void setup() {
  Serial.begin(9600); // Comunicação com ESP32
  delay(1000);
  
  Serial.println("=== ARDUINO LIXEIRA INTELIGENTE ===");
  Serial.println("Inicializando...");
  
  // Configurar servo
  servo.attach(servoPin);
  servo.write(closeAngle);
  lidOpen = false;
  
  // Configurar sensores
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  // Configurar LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  Serial.println("✅ Sistema inicializado!");
  Serial.println("Aguardando comandos...");
  
  // Enviar status inicial
  sendStatus("fechada");
  
  delay(1000);
}

void loop() {
  // Verificar comandos do ESP32 com prioridade
  checkESP32Commands();
  
  // Controle automático apenas se não estiver em modo manual
  if (!manualMode && millis() - lastAutoCheck >= 200) {
    autoControl();
    lastAutoCheck = millis();
  }
  
  // Verificar se deve sair do modo manual
  if (manualMode && millis() - manualModeTimer >= 10000) {
    manualMode = false;
    Serial.println("🔄 Voltando ao modo automático");
  }
  
  // Verificação de nível da lixeira
  if (millis() - lastSensorRead >= 1000) {
    checkTrashLevel();
    lastSensorRead = millis();
  }
  
  // Enviar status periodicamente
  if (millis() - lastStatusSend >= 5000) {
    sendCurrentStatus();
    lastStatusSend = millis();
  }
  
  delay(50); // Delay menor para melhor responsividade
}

void checkESP32Commands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    // Limpar buffer serial
    while (Serial.available()) {
      Serial.read();
    }
    
    Serial.print("📨 Comando recebido: ");
    Serial.println(cmd);
    
    if (cmd == 'O' || cmd == 'o') {
      Serial.println("🔓 Executando comando ABRIR");
      openLid();
      manualMode = true;
      manualModeTimer = millis();
    }
    else if (cmd == 'C' || cmd == 'c') {
      Serial.println("🔒 Executando comando FECHAR");
      closeLid();
      manualMode = true;
      manualModeTimer = millis();
    }
    else {
      Serial.print("❓ Comando desconhecido: ");
      Serial.println((int)cmd);
    }
  }
}

void autoControl() {
  float distance1 = readDistance(trigPin1, echoPin1);
  
  if (distance1 > 0 && distance1 <= 25) { // Objeto próximo
    if (!lidOpen) {
      Serial.println("👋 Presença detectada - abrindo tampa automaticamente");
      openLid();
    }
  } else {
    if (lidOpen) {
      Serial.println("🚶 Objeto removido - fechando tampa automaticamente");
      closeLid();
    }
  }
}

void openLid() {
  if (!lidOpen) {
    Serial.println("🔓 ABRINDO TAMPA...");
    servo.write(openAngle);
    lidOpen = true;
    sendStatus("aberta");
    delay(500); // Aguardar servo se posicionar
    Serial.println("✅ Tampa aberta!");
  } else {
    Serial.println("⚠ Tampa já estava aberta");
  }
}

void closeLid() {
  if (lidOpen) {
    Serial.println("🔒 FECHANDO TAMPA...");
    servo.write(closeAngle);
    lidOpen = false;
    sendStatus("fechada");
    delay(500); // Aguardar servo se posicionar
    Serial.println("✅ Tampa fechada!");
  } else {
    Serial.println("⚠ Tampa já estava fechada");
  }
}

void sendStatus(String status) {
  Serial.println("STATUS:" + status);
  Serial.println("📤 Status enviado para ESP32: " + status);
}

void sendCurrentStatus() {
  String currentStatus = lidOpen ? "aberta" : "fechada";
  sendStatus(currentStatus);
}

void checkTrashLevel() {
  float distance2 = readDistance(trigPin2, echoPin2);
  
  static bool wasFullBefore = false;
  bool isFull = (distance2 > 0 && distance2 <= 14);
  
  if (isFull != wasFullBefore) {
    if (isFull) {
      digitalWrite(ledPin, HIGH);
      Serial.println("🚨 LIXEIRA CHEIA! LED ligado");
      Serial.println("ALERT:lixeira_cheia");
    } else {
      digitalWrite(ledPin, LOW);
      Serial.println("✅ Lixeira não está mais cheia. LED desligado");
      Serial.println("ALERT:lixeira_normal");
    }
    wasFullBefore = isFull;
  }
  
  // Enviar dados do sensor periodicamente
  String sensorData = String(distance2) + "cm";
  if (millis() % 10000 == 0) { // A cada 10 segundos
    Serial.println("SENSOR:" + sensorData);
  }
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // Timeout de 30ms
  
  if (duration == 0) {
    return -1; // Erro na leitura
  }
  
  float distance = duration * 0.034 / 2;
  
  // Filtrar leituras inválidas
  if (distance > 400 || distance < 2) {
    return -1;
  }
  
  return distance;
}