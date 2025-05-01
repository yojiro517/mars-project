#include "servo_maneuver.h"

       
ServoManeuver::ServoManeuver(int left_servo_pin, int right_servo_pin)
    : left_servo_{ServoMotor(left_servo_pin)}, right_servo_{ServoMotor(right_servo_pin)} {
};

void ServoManeuver::init() {
    left_servo_.init();
    right_servo_.init();
}

void ServoManeuver::moveForward(int left_forward_speed, int right_forward_speed) {
    left_servo_.rotateSpeed(left_forward_speed);
    right_servo_.rotateSpeed(right_forward_speed);
}

void ServoManeuver::moveBackward(int leftBackwardSpeed, int rightBackwardSpeed) {
    left_servo_.rotateSpeed(leftBackwardSpeed);
    right_servo_.rotateSpeed(rightBackwardSpeed);
}

void ServoManeuver::turnRight(int left_forward_speed, int right_forward_speedSlow) {
    left_servo_.rotateSpeed(left_forward_speed);
    right_servo_.rotateSpeed(right_forward_speedSlow);
}

void ServoManeuver::turnLeft(int left_forward_speed_slow, int right_forward_speed) {
    left_servo_.rotateSpeed(left_forward_speed_slow);
    right_servo_.rotateSpeed(right_forward_speed);
}

void ServoManeuver::stop(int stop_speed) {
    left_servo_.rotateSpeed(stop_speed);
    right_servo_.rotateSpeed(stop_speed);
}
