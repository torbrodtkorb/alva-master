#ifndef HEIGHT_STEPPER_H
#define HEIGHT_STEPPER_H

#include <stdint.h>

typedef enum {
  DIRECTION_UP,
  DIRECTION_DOWN,
} Direction;

typedef enum {
  SPEED_SLOW,
  SPEED_FAST,
} Speed;

void height_stepper_init ();
void height_stepper_set_direction_up ();
void height_stepper_set_direction_down ();
void height_stepper_move (Direction dir, Speed speed, uint16_t steps);

#endif