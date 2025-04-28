#include "RoverController.h"

// コンストラクタにより変数を定義
RoverController::RoverController(int leftServoPin, int rightServoPin)
    : _leftServoPin(leftServoPin), _rightServoPin(rightServoPin) {}

void RoverController::init() {
    // サーボセットアップ
    _leftServo.attach(_leftServoPin);
    _rightServo.attach(_rightServoPin);
    stopMotors();

    // BME055セットアップ
    setupBno055();
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