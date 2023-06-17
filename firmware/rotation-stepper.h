#ifndef ROTATION_STEPPER_H
#define ROTATION_STEPPER_H

#include <stdbool.h>
#include <stdint.h>

void rotation_stepper_init ();
void rotation_stepper_start ();
void rotation_stepper_stop ();
void rotation_stepper_callback ();
void rotation_stepper_move_back (uint32_t steps);

#endif