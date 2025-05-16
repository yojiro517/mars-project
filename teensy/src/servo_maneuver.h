#include <CanSatSchool.h>
#ifndef SERVOMANEUVER_H
#define SERVOMANEUVER_H

class ServoManeuver {
public:
    ServoManeuver(int left_servo_pin, int right_servo_pin);
    void init();
    void moveForward();
    void moveBackward();
    void turnRight();
    void turnLeft();
    void stop();

private:
    ServoMotor left_servo_;
    ServoMotor right_servo_;
    int stop_speed_               = 0;
    int left_forward_speed_       = 15;
    int left_forward_speed_slow_  = 10;
    int left_backward_speed_      = -10;
    int right_forward_speed_      = -10;
    int right_forward_speed_slow_ = -5;
    int right_backward_speed_     = 10;
};

#endif // SERVOMANEUVER_H
