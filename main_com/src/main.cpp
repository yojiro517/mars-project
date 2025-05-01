#include "RoverController.h"
#include <CanSatSchool.h>

// ピン番号の定義
#define GREEN_LED_PIN (22)        // 緑LEDのピン番号
#define RED_LED_PIN   (23)        // 赤LEDのピン番号
const int LEFT_SERVO_PIN = 2;   // 左サーボモーターのピン番号
const int RIGHT_SERVO_PIN = 3;  // 右サーボモーターのピン番号
const uint8_t BME_I2C_ADDRESS = 0x76; // BME280アドレス
const uint8_t BNO_I2C_ADDRESS = 0x28; // BNO055アドレス

BaroThermoHygrometer bth;
Led green_led{GREEN_LED_PIN};
Led red_led{RED_LED_PIN};

// RoverController オブジェクト作成
RoverController rover(LEFT_SERVO_PIN, RIGHT_SERVO_PIN, BNO_I2C_ADDRESS);

void setup() {
  // デバッグ用シリアル通信
  Serial.begin(115200);
  delay(1000);
  bth.init();

  // LEDの初期化
  green_led.init();
  red_led.init();

  // BME280の初期化
  bth.init();

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
      green_led.blink(1000);
    } else if (command == "R") {
      Serial.println("Action: Blink Red LED");
      red_led.blink(1000);
    } else if (command == "T") {
      Serial.println("Getting data from BME280");
      BaroThermoHygrometer_t bth_data = bth.read();
      String bmeSensorData = "{";
      bmeSensorData += "\"temperature\":" + String(bth_data.temperature, 2) + ",";
      bmeSensorData += "\"pressure\":" + String(bth_data.pressure, 2) + ",";
      bmeSensorData += "\"humidity\":" + String(bth_data.humidity, 2);
      bmeSensorData += "}";
      Serial5.println(bmeSensorData);
      Serial.println(bmeSensorData);
    } else if (command == "E") {
      Serial.println("Getting data from BNO055");
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