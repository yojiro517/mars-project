#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#ifndef ROVER_CONTROLLER_H
#define ROVER_CONTROLLER_H

class RoverController {
public:
    // コンストラクタ
    RoverController(int leftServoPin, int rightServoPin);

    // 初期化関数
    void init();

    // サーボ
    void moveForward();
    void moveBackward();
    void turnRight();
    void turnLeft();
    void stopMotors();

private:
    // サーボ
    int _leftServoPin;
    int _rightServoPin;
    Servo _leftServo;
    Servo _rightServo;
    const int _stopPosition = 90;
    const int _leftForwardSpeed = 105;
    const int _leftForwardSpeedSlow = 100;
    const int _leftBackwardSpeed = 80;
    const int _rightForwardSpeed = 80;
    const int _rightForwardSpeedSlow = 85;
    const int _rightBackwardSpeed = 100;

    const float _seaLevelPressure = 1013.25;
};

#endif 