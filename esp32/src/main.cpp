#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define ESP32_PORT 55555 // ESP32が受信するポート

// WiFi関連
const char *ssid = "ESP32S3Sense";
const char *password = "Password";
WiFiUDP Udp;
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// UARTピン設定
#define UART_TX 1 // ESP32-S3のTXピン
#define UART_RX 2 // ESP32-S3のRXピン
String lastCommand = "";
unsigned long lastCommandTime = 0;
const unsigned long timeout = 500;
uint8_t latestFrame[32];
int latestLen = 0;
float pressure;
float temperature;
float humidity;

void Wifi_setup()
{
  // Wi-Fiアクセスポイントの開始
  if (!WiFi.softAP(ssid, password)) {
    Serial1.println("Failed to start Wi-Fi Access Point");
    return;
  }

  // 固定IP設定
  if (!WiFi.softAPConfig(local_ip, gateway, subnet)) {
    Serial1.println("Failed to configure static IP");
    return;
  }

  // UDP通信の開始
  Udp.begin(ESP32_PORT);

  Serial1.println("WiFi setup is complete");
  Serial1.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
}

void processCommand(const char *command, IPAddress remoteIP, uint16_t remotePort)
{
  static uint8_t recvBuffer[32];
  static int index = 0;
  static bool inFrame = false;

  Serial.begin(115200);
  while (Serial1.available()) {
    uint8_t byte = Serial1.read();
    if (!inFrame) {
      // ヘッダ検出中
      recvBuffer[0] = recvBuffer[1];
      recvBuffer[1] = byte;
      if (recvBuffer[0] == 0x5C && recvBuffer[1] == 0x94) {
        inFrame = true;
        index = 2;
      }
    } else {
      // フレーム収集中
      if (index < sizeof(recvBuffer)) {
        recvBuffer[index] = byte;
        index++;
        if (byte == '\n' && index >= 15) {
          // 完全なフレーム受信
          memcpy(latestFrame, recvBuffer, index);
          latestLen = index;
          inFrame = false;
          index = 0;
          break;  // 一度に1フレームのみ処理
        }
      } else {
        // 長すぎる場合は破棄して再同期
        inFrame = false;
        index = 0;
      }
    }
  }
  // UARTでTeensyに送信
  if (strcmp(command, "W") == 0) {
    Serial1.println("W");
  } else if (strcmp(command, "S") == 0) {
    Serial1.println("S");
  } else if (strcmp(command, "A") == 0) {
    Serial1.println("A");
  } else if (strcmp(command, "D") == 0) {
    Serial1.println("D");
  } else if (strcmp(command, "R") == 0) {
    Serial1.println("R");
  } else if (strcmp(command, "G") == 0) {
    Serial1.println("G");
  } else if (strcmp(command, "T") == 0) {
    Serial1.println("T");
    if (latestLen == 15 &&
      latestFrame[0] == 0x5C &&
      latestFrame[1] == 0x94 &&
      latestFrame[14] == '\n') {
        int mem = 2;
        memcpy(&pressure, &latestFrame[mem], sizeof(float));
        mem = mem + sizeof(float);
        memcpy(&temperature, &latestFrame[mem], sizeof(float));
        mem = mem + sizeof(float);
        memcpy(&humidity, &latestFrame[mem], sizeof(float));
        Serial.println("pressure");
        Serial.println(pressure);
        Serial.println("temperture");
        Serial.println(temperature);
        Serial.println("humidity");
        Serial.println(humidity);
        Udp.beginPacket(remoteIP, remotePort);
        Udp.write(&latestFrame[2], 12);
        Udp.endPacket();
    }
  } else if (strcmp(command, "B") == 0) {
      Serial1.println("B");
  } else {
    Serial1.println("Unknown command received.");
  }
}

void setup()
{
  Serial.begin(115200); // デバッグ用シリアル通信
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX); // UART通信開始
  delay(1000);
  Wifi_setup(); // Wi-Fi設定
  Serial1.println("Ready to receive continuous commands via UDP.");
}

void loop()
{
  // UDPデータ受信処理
  int packetSize = Udp.parsePacket(); // 受信パケットサイズを取得（受信がなかったら０）
  if (packetSize > 0) {
    char packetBuffer[255];
    IPAddress remoteIP = Udp.remoteIP();
    uint16_t remotePort = Udp.remotePort();
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) {
      packetBuffer[len] = '\0';
      lastCommand = packetBuffer;
      lastCommandTime = millis();

      // コマンドを処理
      processCommand(packetBuffer, remoteIP, remotePort);
    }
  }
  // コマンド維持タイムアウト処理
  if (millis() - lastCommandTime > timeout) {
    lastCommand = "B";
    processCommand("B", IPAddress(), 0);
    lastCommandTime = millis();
  }
  delay(1);
}
