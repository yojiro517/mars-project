#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BNO055.h>
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

    // 温度・気圧センサ BME280
    void setupLED();
    void setupBme280();
    String getBmeData();

    void motion(String, RoverController);

private:
    // LED
    static constexpr int GREEN_LED_PIN = 22;   // 緑LED
    static constexpr int RED_LED_PIN = 23;     // 赤LED
    Led green_led{GREEN_LED_PIN};
    Led red_led{RED_LED_PIN};

    // サーボ
    static constexpr int LEFT_SERVO_PIN = 2;   // 左サーボ
    static constexpr int RIGHT_SERVO_PIN = 3;  // 右サーボ
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

    // 温度・気圧センサ BME280
    BaroThermoHygrometer bth;
};

#endif 