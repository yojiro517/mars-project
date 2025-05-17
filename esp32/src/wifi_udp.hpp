#pragma once

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>

#define ESP32_PORT 55555 // ESP32が受信するポート
#define TELEMETRY_SIZE (1440)

class WifiUdp : public WiFiUDP
{
  public:
    WifiUdp(char const* ssid, char const* password);
    ~WifiUdp() = default;

    void init();
    void send(const char *ip, int port, const uint8_t *data, size_t packet_num);

  private:
    const char* ssid_;
    const char* password_;
    IPAddress local_ip_ = IPAddress(192, 168, 1, 1);
    IPAddress gateway_ = IPAddress(192, 168, 1, 1);
    IPAddress subnet_ = IPAddress(255, 255, 255, 0);
};