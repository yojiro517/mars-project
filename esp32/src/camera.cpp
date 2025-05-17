#include "camera.hpp"

#include <Arduino.h>

Camera::Camera()
{
  // Constructor
}

void Camera::init()
{
  // カメラ初期化
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_240X240;
  config.pixel_format = PIXFORMAT_RGB565;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_LATEST;
  esp_camera_init(&config);

  Serial.printf("Camera setup is complete\n");
}

void Camera::send_photo(const char *IP, int PORT, WifiUdp &wifiUdp)
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
    uint8_t p_r[IMAGE_SIZE] = {};
    uint8_t p_g[IMAGE_SIZE] = {};
    uint8_t p_b[IMAGE_SIZE] = {};
    int j = 0;
    for (j=0; j < IMAGE_SIZE; j++){
      uint16_t p0 = p[IMAGE_SIZE*4*i+4*j+0]*256 + p[IMAGE_SIZE*4*i+4*j+1];
      uint16_t p1 = p[IMAGE_SIZE*4*i+4*j+2]*256 + p[IMAGE_SIZE*4*i+4*j+3];
      p_r[j] = ((p0 & (uint16_t)0b1111000000000000)>>8) | ((p1 & (uint16_t)0b1111000000000000)>>12);
      p_g[j] = ((p0 & (uint16_t)0b0000011110000000)>>3) | ((p1 & (uint16_t)0b0000011110000000)>>7);
      p_b[j] = ((p0 & (uint16_t)0b0000000000011110)<<3) | ((p1 & (uint16_t)0b0000000000011110)>>1);
    }
    wifiUdp.send(IP, PORT, p_r, 3*i+1);
    wifiUdp.send(IP, PORT, p_g, 3*i+2);
    wifiUdp.send(IP, PORT, p_b, 3*i+3);
  }
}

