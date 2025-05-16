#include <CanSatSchool.h>
#include "servo_maneuver.h"

// ピン番号の定義
#define GREEN_LED_PIN (22)    // 緑LEDのピン番号
#define RED_LED_PIN   (23)    // 赤LEDのピン番号
#define LEFT_SERVO_PIN  (2)   // 左サーボモーターのピン番号
#define RIGHT_SERVO_PIN (3)   // 右サーボモーターのピン番号

BaroThermoHygrometer bth;
Led green_led{GREEN_LED_PIN};
Led red_led{RED_LED_PIN};
ServoManeuver servo_maneuver(LEFT_SERVO_PIN, RIGHT_SERVO_PIN);

void command_execute(String command);
void send_bth_data();

void setup() {
  // デバッグ用シリアル通信
  Serial.begin(115200);
  delay(1000);
  Serial.println("Teensy UART Receiver Started");
  Wire.begin();

  //UART通信用開始
  Serial5.begin(115200);

  // LEDの初期化
  green_led.init();
  red_led.init();

  // BME280の初期化
  bth.init(); 

  // rover初期関数実行
  servo_maneuver.init();
}

void loop() {
  // UARTでデータを受信
  if (Serial5.available() > 0) {
    String command = Serial5.readStringUntil('\n');
    command.trim();

    // 受信データを表示（デバッグ用）
    Serial.print("Received Command: ");
    Serial.println(command);

    // コマンドに基づいて処理を実行
    command_execute(command);
  }

  delay(100);
}

void command_execute(String command) {
  if (command == "W") {
      Serial.println("Action: Move Forward");
      servo_maneuver.moveForward();
  } else if (command == "S") {
      Serial.println("Action: Move Backward");
      servo_maneuver.moveBackward();
  } else if (command == "A") {
      Serial.println("Action: Turn Left");
      servo_maneuver.turnLeft();
  } else if (command == "D") {
      Serial.println("Action: Turn Right");
      servo_maneuver.turnRight();
  } else if (command == "G") {
      Serial.println("Action: Blink Green LED");
      green_led.blink(1000);
  } else if (command == "R") {
      Serial.println("Action: Blink Red LED");
      red_led.blink(1000);
  } else if (command == "B") {
      Serial.println("Action: Stopping");
      servo_maneuver.stop();
  } else {
      Serial.println("Unknown Command");
  }
  //常に行う処理
  send_bth_data();
}

void send_bth_data() {
    BaroThermoHygrometer_t bth_data = bth.read();
    char bmeSensorData[100] = "";
    int len = 0;
    bmeSensorData[len] = 0x5C;
    len++;
    bmeSensorData[len] = 0x94;
    len++;
    memcpy(&bmeSensorData[len], &bth_data, sizeof(bth_data));
    len += sizeof(bth_data);
    bmeSensorData[len] = '\n';
    len++;
    Serial5.write(bmeSensorData, len);
}
