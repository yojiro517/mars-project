#include <CanSatSchool.h>
#ifndef SERVOMANEUVER_H
#define SERVOMANEUVER_H

class ServoManeuver {
public:
    ServoManeuver(int left_servo_pin, int right_servo_pin);
    void init();
    void moveForward(int left_speed, int right_speed);
    void moveBackward(int left_speed, int right_speed);
    void turnRight(int left_speed, int right_speed);
    void turnLeft(int left_speed, int right_speed);
    void stop(int stop_speed);

private:
    ServoMotor left_servo_;
    ServoMotor right_servo_;
};

#endif 