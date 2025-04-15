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
    RoverController(int leftServoPin, int rightServoPin, uint8_t bmeI2cAddress, uint8_t bnoI2cAddress);

    // 初期化関数
    void init();

    // サーボ
    void moveForward();
    void moveBackward();
    void turnRight();
    void turnLeft();
    void stopMotors();

    // 温度・気圧センサ BME280
    void setupBme280();
    String getBmeData();

    //　9軸センサ BNO055
    void setupBno055();
    String getBnoEulerData();
    String getBno9AxisData();

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

    // 温度・気圧センサ BME280
    Adafruit_BME280 _bme;
    uint8_t _bmeI2cAddress;
    const float _seaLevelPressure = 1013.25;

    // 9軸センサ BNO055
    uint8_t _bnoI2cAddress;
    Adafruit_BNO055 bno = Adafruit_BNO055(-1, _bnoI2cAddress, &Wire);
};

#endif 