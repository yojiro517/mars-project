#include <CanSatSchool.h>
#include "servo_maneuver.h"
#include "loop_func.h"
#include "hardware.h"

// ピン番号の定義
#define GREEN_LED_PIN (22)    // 緑LEDのピン番号
#define RED_LED_PIN   (23)    // 赤LEDのピン番号
#define LEFT_SERVO_PIN  (2)   // 左サーボモーターのピン番号
#define RIGHT_SERVO_PIN (3)   // 右サーボモーターのピン番号

hardware_bundle hb = {
  BaroThermoHygrometer(),
  Led(GREEN_LED_PIN),
  Led(RED_LED_PIN),
  ServoManeuver(LEFT_SERVO_PIN, RIGHT_SERVO_PIN)
};

void setup() {
  // デバッグ用シリアル通信
  Serial.begin(115200);
  delay(1000);
  hb.bth.init();

  // LEDの初期化
  hb.green_led.init();
  hb.red_led.init();

  // BME280の初期化
  hb.bth.init();

  //UART通信用開始
  Serial5.begin(115200);
  Serial.println("Teensy UART Receiver Started");

  // rover初期関数実行
  hb.servo_maneuver.init();
}

void loop() {
  // UARTでデータを受信
  if (Serial5.available() > 0) {
    String command = Serial5.readStringUntil('\n');
    command.trim();

    // 受信データを表示（デバッグ用）
    Serial.print("Received Command: ");
    Serial.println(command);

    command_execute(command, hb);
  }

  delay(100);
}