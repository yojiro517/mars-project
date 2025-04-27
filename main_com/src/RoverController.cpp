#include "RoverController.h"
#include <CanSatSchool.h>

// コンストラクタにより変数を定義
RoverController::RoverController(int leftServoPin, int rightServoPin)
    : _leftServoPin(leftServoPin), _rightServoPin(rightServoPin) {}

void RoverController::init() {
    // ledセットアップ
    setupLED();

    // サーボセットアップ
    _leftServo.attach(_leftServoPin);
    _rightServo.attach(_rightServoPin);
    stopMotors();

    // BME280セットアップ
    setupBme280();
}

// サーボ関連の関数
void RoverController::moveForward() {
    _leftServo.write(_leftForwardSpeed);
    _rightServo.write(_rightForwardSpeed);
}

void RoverController::moveBackward() {
    _leftServo.write(_leftBackwardSpeed);
    _rightServo.write(_rightBackwardSpeed);
}

void RoverController::turnRight() {
    _leftServo.write(_leftForwardSpeed);
    _rightServo.write(_rightForwardSpeedSlow);
}

void RoverController::turnLeft() {
    _leftServo.write(_leftForwardSpeedSlow);
    _rightServo.write(_rightForwardSpeed);
}

void RoverController::stopMotors() {
    _leftServo.write(_stopPosition);
    _rightServo.write(_stopPosition);
}

void RoverController::setupLED() {
    green_led.init();
    red_led.init();
    Serial.println("LED initialized successfully");
}

void RoverController::setupBme280() {
    bth.init();
    Serial.println("BME280 initialized successfully.");
}

String RoverController::getBmeData() {
    float temperature = bth.read().temperature;  // [°C]
    float pressure = bth.read().pressure;  // [hPa]
    float humidity = bth.read().humidity;  // [%]
    String bmeSensorData = "{";
    bmeSensorData += "\"temperature\":" + String(temperature, 2) + ",";
    bmeSensorData += "\"pressure\":" + String(pressure, 2) + ",";
    bmeSensorData += "\"humidity\":" + String(humidity, 2);
    bmeSensorData += "}";
    return bmeSensorData;
}

void RoverController::motion(String command, RoverController rover) {
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
      green_led.blink(1000); // 1秒間隔で点滅
    } else if (command == "R") {
      Serial.println("Action: Blink Red LED");
      red_led.blink(1000); // 1秒間隔で点滅
    } else if (command == "T") {
      Serial.println("Getting data from BME280");
      String bmeSensorData = rover.getBmeData();
      Serial5.println(bmeSensorData);
      Serial.println(bmeSensorData);
    } else if (command == "B") {
      Serial.println("Action: Stopping");
      rover.stopMotors();
    } else {
      Serial.println("Unknown Command");
    }
}