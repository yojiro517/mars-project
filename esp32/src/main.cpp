#include <Arduino.h>

#include <WebServer.h>

#include <Wire.h>

#include "camera.hpp"
#include "wifi_udp.hpp"

// #define DEBUG_MODE
// #define USE_CAMERA

#define CONSOLE_IP "192.168.1.2"
#define CONSOLE_PORT 50000

// WiFi関連
const char *ssid = "ESP32S3Sense";
const char *password = "Password";
WifiUdp wifiUdp(ssid, password);
WebServer server(80);
Camera camera;

#ifdef DEBUG_MODE
#define SerialComm Serial
#else
#define SerialComm Serial1
#endif

// UARTピン設定
#define UART_TX 1 // ESP32-S3のTXピン
#define UART_RX 2 // ESP32-S3のRXピン
String lastCommand = "";
unsigned long lastCommandTime = 0;
const unsigned long timeout = 200;
uint8_t latestFrame[32];
int latestLen = 0;
float pressure;
float temperature;
float humidity;

uint8_t dummy_telem[IMAGE_SIZE] = {};

uint8_t command_counter = 0;
const uint32_t buffer_size = 1024;
char command[buffer_size] = {};

void process_command(const char *command, IPAddress remoteIP, uint16_t remotePort);
void send_bth_data(uint8_t* packet);

void setup()
{
  Serial.begin(115200); // デバッグ用シリアル通信
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX); // UART通信開始
  delay(1000);

  wifiUdp.init();
  // Serverの開始
  server.begin();

  #ifdef USE_CAMERA
  camera.init();
  #endif

  Serial1.println("Ready to receive continuous commands via UDP.");
}

void loop()
{
  // UDPデータ受信処理
  int packetSize = wifiUdp.parsePacket(); // 受信パケットサイズを取得（受信がなかったら０）
  if (packetSize > 0) {
    char packetBuffer[255];
    IPAddress remoteIP = wifiUdp.remoteIP();
    uint16_t remotePort = wifiUdp.remotePort();
    int len = wifiUdp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) {
      packetBuffer[len] = '\0';
      lastCommand = packetBuffer;
      lastCommandTime = millis();

      // コマンドを処理
      process_command(packetBuffer, remoteIP, remotePort);
    }
  }
  // コマンド維持タイムアウト処理
  if (millis() - lastCommandTime > timeout) {
    lastCommand = "B";
    process_command("B", IPAddress(), 0);
    lastCommandTime = millis();
#ifdef USE_CAMERA
    camera.send_photo(CONSOLE_IP, CONSOLE_PORT, wifiUdp);
    wifiUdp.send(CONSOLE_IP, CONSOLE_PORT, dummy_telem, 0xFF);
#endif
  }
  delay(1);
}

void process_command(const char *command, IPAddress remoteIP, uint16_t remotePort)
{
  static uint8_t recvBuffer[32];
  static int index = 0;
  static bool inFrame = false;

  Serial.begin(115200);
  while (SerialComm.available()) {
    uint8_t byte = SerialComm.read();
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
    if (latestLen == 15 &&
      latestFrame[0] == 0x5C &&
      latestFrame[1] == 0x94 &&
      latestFrame[14] == '\n') {
        send_bth_data(latestFrame);
        wifiUdp.send_data(remoteIP, remotePort, &latestFrame[0], 15);
    }
  } else if (strcmp(command, "B") == 0) {
      Serial1.println("B");
  } else {
    Serial1.println("Unknown command received.");
  }
}

void send_bth_data(uint8_t* packet)
{
  int mem = 2;
  memcpy(&pressure, &packet[mem], sizeof(float));
  mem = mem + sizeof(float);
  memcpy(&temperature, &packet[mem], sizeof(float));
  mem = mem + sizeof(float);
  memcpy(&humidity, &packet[mem], sizeof(float));
  SerialComm.println("pressure");
  SerialComm.println(pressure);
  SerialComm.println("temperture");
  SerialComm.println(temperature);
  SerialComm.println("humidity");
  SerialComm.println(humidity);
}
