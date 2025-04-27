#include "RoverController.h"
#include <CanSatSchool.h>

// ピン番号の定義
const int LEFT_SERVO_PIN = 2;   // 左サーボモーターのピン番号
const int RIGHT_SERVO_PIN = 3;  // 右サーボモーターのピン番号


// RoverController オブジェクト作成
RoverController rover(LEFT_SERVO_PIN, RIGHT_SERVO_PIN);

void setup() {
  // デバッグ用シリアル通信
  Serial.begin(115200);
  delay(1000);

  //UART通信用開始
  Serial5.begin(115200);
  Serial.println("Teensy UART Receiver Started");

  // rover初期関数実行
  rover.init();
}

void loop() {
  // UARTでデータを受信
  if (Serial5.available() > 0) {
    String command = Serial5.readStringUntil('\n');
    command.trim();

    // 受信データを表示（デバッグ用）
    Serial.print("Received Command: ");
    Serial.println(command);

    rover.motion(command, rover);
  }

  delay(100);
}