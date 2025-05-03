#include "loop_func.h"

void command_execute(String command, hardware_bundle& hb) {
    if (command == "W") {
        Serial.println("Action: Move Forward");
        hb.servo_maneuver.moveForward();
    } else if (command == "S") {
        Serial.println("Action: Move Backward");
        hb.servo_maneuver.moveBackward();
    } else if (command == "A") {
        Serial.println("Action: Turn Left");
        hb.servo_maneuver.turnLeft();
    } else if (command == "D") {
        Serial.println("Action: Turn Right");
        hb.servo_maneuver.turnRight();
    } else if (command == "G") {
        Serial.println("Action: Blink Green LED");
        hb.green_led.blink(1000);
    } else if (command == "R") {
        Serial.println("Action: Blink Red LED");
        hb.red_led.blink(1000);
    } else if (command == "T") {
        Serial.println("Getting data from BME280");
        BaroThermoHygrometer_t bth_data = hb.bth.read();
        String bmeSensorData = "{";
        bmeSensorData += "\"temperature\":" + String(bth_data.temperature, 2) + ",";
        bmeSensorData += "\"pressure\":" + String(bth_data.pressure, 2) + ",";
        bmeSensorData += "\"humidity\":" + String(bth_data.humidity, 2);
        bmeSensorData += "}";
        Serial5.println(bmeSensorData);
        Serial.println(bmeSensorData);
    } else if (command == "B") {
        Serial.println("Action: Stopping");
        hb.servo_maneuver.stop();
    } else {
        Serial.println("Unknown Command");
    }
}