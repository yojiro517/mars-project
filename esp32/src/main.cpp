#include <Arduino.h>

#include <WebServer.h>

#include <Wire.h>

#include "camera.hpp"
#include "wifi_udp.hpp"

// #define DEBUG_MODE

#define CONSOLE_IP "192.168.1.2"
#define CONSOLE_PORT 50000

// WiFi関連
const char *ssid = "ESP32S3Sense";
const char *password = "Password";
WiFiUDP Udp;
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

bool camera_enable = true;

const int telem_size = 1440;
uint8_t dummy_telem[telem_size] = {};

uint8_t command_counter = 0;
const uint32_t buffer_size = 1024;
char command[buffer_size] = {};


void send_photo(const char *IP, int PORT);
void process_command(const char *command, IPAddress remoteIP, uint16_t remotePort);
void send_bth_data(uint8_t* packet);

void setup()
{
  Serial.begin(115200); // デバッグ用シリアル通信
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX); // UART通信開始
  delay(1000);
  camera_enable = true;

  wifiUdp.init();
  // Serverの開始
  server.begin();
  camera.init();

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
      process_command(packetBuffer, remoteIP, remotePort);
    }
  }
  // コマンド維持タイムアウト処理
  if (millis() - lastCommandTime > timeout) {
    lastCommand = "B";
    process_command("B", IPAddress(), 0);
    lastCommandTime = millis();
    send_photo(CONSOLE_IP, CONSOLE_PORT);
    wifiUdp.send(CONSOLE_IP, CONSOLE_PORT, dummy_telem, 0xFF);
  }
  delay(1);
}

void send_photo(const char *IP, int PORT)
{
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to get camera frame buffer");
    return;
  }
  esp_camera_fb_return(fb);

  uint8_t *p = fb->buf;

  int i = 0;
  for (i=0; i<20; i++){
    uint8_t p_r[1440] = {};
    uint8_t p_g[1440] = {};
    uint8_t p_b[1440] = {};
    int j = 0;
    for (j=0; j<1440; j++){
      uint16_t p0 = p[1440*4*i+4*j+0]*256 + p[1440*4*i+4*j+1];
      uint16_t p1 = p[1440*4*i+4*j+2]*256 + p[1440*4*i+4*j+3];
      p_r[j] = ((p0 & (uint16_t)0b1111000000000000)>>8) | ((p1 & (uint16_t)0b1111000000000000)>>12);
      p_g[j] = ((p0 & (uint16_t)0b0000011110000000)>>3) | ((p1 & (uint16_t)0b0000011110000000)>>7);
      p_b[j] = ((p0 & (uint16_t)0b0000000000011110)<<3) | ((p1 & (uint16_t)0b0000000000011110)>>1);
    }
    wifiUdp.send(IP, PORT, p_r, 3*i+1);
    wifiUdp.send(IP, PORT, p_g, 3*i+2);
    wifiUdp.send(IP, PORT, p_b, 3*i+3);
  }
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
        Udp.beginPacket(remoteIP, remotePort);
        Udp.write(&latestFrame[0], 15);
        Udp.endPacket();
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
