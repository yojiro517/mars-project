#include "RoverController.h"

// ピン番号の定義
const int TEENSY_LED_PIN = LED_BUILTIN;   // Teensyの内蔵LEDピン（今回は使っていない）
const int GREEN_LED_PIN = 22;   // 緑LEDのピン番号
const int RED_LED_PIN = 23;     // 赤LEDのピン番号
const int LEFT_SERVO_PIN = 2;   // 左サーボモーターのピン番号
const int RIGHT_SERVO_PIN = 3;  // 右サーボモーターのピン番号
const uint8_t BME_I2C_ADDRESS = 0x76; // BME280アドレス
const uint8_t BNO_I2C_ADDRESS = 0x28; // BNO055アドレス

// RoverController オブジェクト作成
RoverController rover(TEENSY_LED_PIN, GREEN_LED_PIN, RED_LED_PIN, LEFT_SERVO_PIN, RIGHT_SERVO_PIN, BME_I2C_ADDRESS, BNO_I2C_ADDRESS);

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

    if (command == "W") {
      Serial.println("Action: Move Forward");
      rover.moveForward();
    } else if (command == "S") {
      Serial.println("Action: Move Backward");
      rover.moveBackward();
    } else if (command == "A") {
      Serial.println("Action: Turn Left");
      rover.turnLeft();
    } else if (command == "D") {
      Serial.println("Action: Turn Right");
      rover.turnRight();
    } else if (command == "G") {
      Serial.println("Action: Blink Green LED");
      rover.blinkGreenLed();
    } else if (command == "R") {
      Serial.println("Action: Blink Red LED");
      rover.blinkRedLed();
    } else if (command == "T") {
      Serial.println("Getting data from BME280");
      String bmeSensorData = rover.getBmeData();
      Serial5.println(bmeSensorData);
      Serial.println(bmeSensorData);
    } else if (command == "E") {
      Serial.println("Getting data from BME280");
      String bnoSensorData = rover.getBnoEulerData();
      Serial5.println(bnoSensorData);
      Serial.println(bnoSensorData);
    } else if (command == "B") {
      Serial.println("Action: Stopping");
      rover.stopMotors();
    } else {
      Serial.println("Unknown Command");
    }
  }

  delay(100);
}