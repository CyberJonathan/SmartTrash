# 📝 **README - Lixeira Inteligente com ESP32 e Firebase**  


## 🗑️ **Descrição do Projeto**  
Este projeto consiste em uma **lixeira inteligente** controlada remotamente via **WiFi**, utilizando um **ESP32** para comunicação com o **Firebase** e um **Arduino** para controle dos atuadores (como servo motores para abrir/fechar a tampa).  

**Funcionalidades principais:**  
✅ **Controle remoto** (abrir/fechar via Firebase)  
✅ **Monitoramento em tempo real** (status da tampa)  
✅ **Sensores integrados** (detecção de proximidade, nível de lixo)  
✅ **Alertas automáticos** (lixeira cheia, mau funcionamento)  

---

## 🛠 **Tecnologias Utilizadas**  

| Componente          | Descrição                                                                 |
|---------------------|--------------------------------------------------------------------------|
| **ESP32**           | Microcontrolador WiFi para comunicação com a internet                    |
| **Arduino**         | Controla os atuadores e sensores locais                                  |
| **Firebase Realtime Database** | Armazena comandos e status em tempo real                     |
| **Hardware Serial** | Comunicação entre ESP32 e Arduino                                        |
| **WiFi Manager**    | Facilita a conexão com redes WiFi                                        |

---

## 📡 **Como Funciona?**  

### 🔄 **Fluxo de Comunicação**  
1. **Aplicativo Web/Firebase** envia um comando (`abrir`/`fechar`).  
2. **ESP32** verifica periodicamente o Firebase em busca de novos comandos.  
3. **Arduino** recebe o comando via **Serial (UART)** e aciona o motor/servo.  
4. **Sensores** enviam dados (nível de lixo, status da tampa) de volta para o Firebase.  



## ⚙️ **Configuração do Projeto**  

### 📋 **Pré-requisitos**  
- **ESP32** (com suporte a Arduino IDE)  
- **Arduino Uno/Nano** (para controle local)  
- **Conta no Firebase** (banco de dados em tempo real)  
- **Conexão WiFi** (2.4GHz)  

### 🔧 **Instalação**  
1. **Clone o repositório:**  
   ```bash
   git clone https://github.com/seu-usuario/lixeira-inteligente.git
   cd lixeira-inteligente
   ```

2. **Configure o Firebase:**  
   - Crie um projeto no [Firebase Console](https://console.firebase.google.com/).  
   - Ative o **Realtime Database** e adicione as credenciais ao código.  

3. **Carregue o código no ESP32:**  
   - Abra o projeto no **Arduino IDE** ou **PlatformIO**.  
   - Instale as bibliotecas necessárias:  
     ```cpp
     #include <WiFi.h>
     #include <HTTPClient.h>
     #include <ArduinoJson.h>
     ```
   - Modifique `WIFI_SSID`, `WIFI_PASSWORD` e `FIREBASE_URL`.  

4. **Conecte o Arduino:**  
   - O ESP32 se comunica via **pinos RX/TX (UART2)**.  
   - Certifique-se de que o Arduino está programado para receber comandos `'O'` (abrir) e `'C'` (fechar).  

---
