#include "wifi_udp.hpp"

#include <Arduino.h>

WifiUdp::WifiUdp(char const* ssid, char const* password) : ssid_(ssid), password_(password), WiFiUDP()
{
  // Constructor
}

void WifiUdp::init()
{
  // Wi-Fiアクセスポイントの開始
  if (!WiFi.softAP(ssid_, password_)) {
    Serial.println("Failed to start Wi-Fi Access Point");
    return;
  }

  // 固定IP設定
  if (!WiFi.softAPConfig(local_ip_, gateway_, subnet_))
  {
    Serial.println("Failed to configure static IP");
    return;
  }

  // Serverの開始
  // server.begin();

  // UDP通信の開始
  WifiUdp::begin(ESP32_PORT);
  
  delay(500);
  WiFi.setTxPower(WIFI_POWER_15dBm);

  Serial.println("WiFi setup is complete");
  Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
}

void WifiUdp::send(const char *ip, int port, const uint8_t *data, size_t packet_num)
{
  int i = 0;
  WifiUdp::beginPacket(ip, port);
  WifiUdp::write(packet_num);
  uint8_t sum=0;
  for (i=0; i < TELEMETRY_SIZE; i++) {
      WifiUdp::write(data[i]);
      sum += data[i];
  }
  WifiUdp::write((uint8_t)sum);
  WifiUdp::endPacket();
  delay(1);
}