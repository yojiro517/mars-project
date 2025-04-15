#include "RoverController.h"

// コンストラクタにより変数を定義
RoverController::RoverController(int leftServoPin, int rightServoPin, uint8_t bmeI2cAddress, uint8_t bnoI2cAddress)
    : _leftServoPin(leftServoPin), _rightServoPin(rightServoPin), _bmeI2cAddress(bmeI2cAddress), _bnoI2cAddress(bnoI2cAddress) {}

void RoverController::init() {
    // サーボセットアップ
    _leftServo.attach(_leftServoPin);
    _rightServo.attach(_rightServoPin);
    stopMotors();

    // BME280セットアップ
    setupBme280();

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

// 温度・気圧センサ関連の関数
void RoverController::setupBme280() {
    if (!_bme.begin(_bmeI2cAddress)) {
        Serial.println("Could not find BME280 sensor. Check wiring or I2C address.");
        while (1); // エラーの場合は無限ループ
    }
    Serial.println("BME280 initialized successfully.");
}

String RoverController::getBmeData() {
    float temperature = _bme.readTemperature();
    float pressure = _bme.readPressure();
    float altitude = _bme.readAltitude(_seaLevelPressure);
    float humidity = _bme.readHumidity();
    String bmeSensorData = "{";
    bmeSensorData += "\"temperature\":" + String(temperature, 2) + ",";
    bmeSensorData += "\"pressure\":" + String(pressure, 2) + ",";
    bmeSensorData += "\"altitude\":" + String(altitude, 2) + ",";
    bmeSensorData += "\"humidity\":" + String(humidity, 2);
    bmeSensorData += "}";
    return bmeSensorData;
}

// 9軸センサ関連の関数
void RoverController::setupBno055() {
    if (!bno.begin()) {
        Serial.println("Could not find BNO055 sensor. Check wiring or I2C address.");
        while(1);
    }
    Serial.println("BNO055 initialized successfully.");
}

String RoverController::getBnoEulerData() {
    //　オイラー角の取得
    double yaw = bno.getVector(Adafruit_BNO055::VECTOR_EULER).z();
    double pitch = bno.getVector(Adafruit_BNO055::VECTOR_EULER).y();
    double roll = bno.getVector(Adafruit_BNO055::VECTOR_EULER).x();

    // キャリブレーション状況の取得
    uint8_t system, gyro, accel, mag = 0;
    bno.getCalibration(&system, &gyro, &accel, &mag);

    String bnoSensorData = "{";
    bnoSensorData += "\"system\":" + String(system) + ",";
    bnoSensorData += "\"gyro\":" + String(gyro) + ",";
    bnoSensorData += "\"accel\":" + String(accel) + ",";
    bnoSensorData += "\"mag\":" + String(mag) + ",";
    bnoSensorData += "\"yaw\":" + String(yaw, 2) + ",";
    bnoSensorData += "\"pitch\":" + String(pitch, 2) + ",";
    bnoSensorData += "\"roll\":" + String(roll, 2) + ",";
    bnoSensorData += "}";
    return bnoSensorData;
}

String RoverController::getBno9AxisData() {
    // 加速度データの取得（単位: m/s^2）
    imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    double accelX = accel.x();
    double accelY = accel.y();
    double accelZ = accel.z();

    // ジャイロデータの取得（単位: deg/s）
    imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    double gyroX = gyro.x();
    double gyroY = gyro.y();
    double gyroZ = gyro.z();

    // 磁気データの取得（単位: µT）
    imu::Vector<3> mag = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    double magX = mag.x();
    double magY = mag.y();
    double magZ = mag.z();

    // キャリブレーション状況の取得
    uint8_t system, gyroCalib, accelCalib, magCalib;
    bno.getCalibration(&system, &gyroCalib, &accelCalib, &magCalib);

    String bnoSensorData = "{";
    bnoSensorData += "\"accel\":{\"x\":" + String(accelX, 2) + ",\"y\":" + String(accelY, 2) + ",\"z\":" + String(accelZ, 2) + "},";
    bnoSensorData += "\"gyro\":{\"x\":" + String(gyroX, 2) + ",\"y\":" + String(gyroY, 2) + ",\"z\":" + String(gyroZ, 2) + "},";
    bnoSensorData += "\"mag\":{\"x\":" + String(magX, 2) + ",\"y\":" + String(magY, 2) + ",\"z\":" + String(magZ, 2) + "},";
    bnoSensorData += "\"calibration\":{\"system\":" + String(system) + ",\"gyro\":" + String(gyroCalib) + ",\"accel\":" + String(accelCalib) + ",\"mag\":" + String(magCalib) + "}";
    bnoSensorData += "}";

    return bnoSensorData;
}
