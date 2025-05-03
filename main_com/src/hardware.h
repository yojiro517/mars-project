#ifndef HARDWARE_H
#define HARDWARE_H
#include <CanSatSchool.h>
#include "servo_maneuver.h"

struct hardware_bundle {
    BaroThermoHygrometer bth;
    Led green_led;
    Led red_led;
    ServoManeuver servo_maneuver;
};

#endif