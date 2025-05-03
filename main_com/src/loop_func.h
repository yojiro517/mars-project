#include <Arduino.h>
#include "servo_maneuver.h"
#include <CanSatSchool.h>
#include "hardware.h"
# ifndef LOOPFUNC_H
# define LOOPFUNC_H

void command_execute(String command, hardware_bundle& hb);

#endif