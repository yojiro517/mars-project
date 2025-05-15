#include "servo_maneuver.h"

       
ServoManeuver::ServoManeuver(int left_servo_pin, int right_servo_pin)
    : left_servo_{ServoMotor(left_servo_pin)}, right_servo_{ServoMotor(right_servo_pin)} {
};

void ServoManeuver::init() {
    left_servo_.init();
    right_servo_.init();
    delay(10);
    stop();
}

void ServoManeuver::moveForward() {
    left_servo_.rotateSpeed(left_forward_speed_);
    right_servo_.rotateSpeed(right_forward_speed_);
}

void ServoManeuver::moveBackward() {
    left_servo_.rotateSpeed(left_backward_speed_);
    right_servo_.rotateSpeed(right_backward_speed_);
}

void ServoManeuver::turnRight() {
    left_servo_.rotateSpeed(left_forward_speed_);
    right_servo_.rotateSpeed(right_forward_speed_slow_);
}

void ServoManeuver::turnLeft() {
    left_servo_.rotateSpeed(left_forward_speed_slow_);
    right_servo_.rotateSpeed(right_forward_speed_);
}

void ServoManeuver::stop() {
    left_servo_.rotateSpeed(stop_speed_);
    right_servo_.rotateSpeed(stop_speed_);
}
