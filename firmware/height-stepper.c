#include <height-stepper.h>
#include <uart.h>
#include <avr/io.h>
#include <util/delay.h>

int32_t stepper_height = -1;

void height_stepper_init () {
  PORTD.DIRSET = 1 << 2 | 1 << 3;
}

void height_stepper_set_direction_up () {
  PORTD.OUTSET = 1 << 3;
}

void height_stepper_set_direction_down () {
  PORTD.OUTCLR = 1 << 3;
}

static inline void height_stepper_step () {
  PORTD.OUTTGL = 1 << 2;
}

void height_stepper_move (Direction direction, Speed speed, uint16_t steps) {
  if (direction == DIRECTION_DOWN)
    height_stepper_set_direction_down();
  else
    height_stepper_set_direction_up();
  
  if (speed == SPEED_FAST) {
    for (uint16_t i = 0; i < steps; i++) {
      height_stepper_step();
      _delay_us(50);
      height_stepper_step();
      _delay_us(50);

      if (abort_flag) return;
    }
  } else {
    for (uint16_t i = 0; i < steps; i++) {
      height_stepper_step();
      _delay_us(500);
      height_stepper_step();
      _delay_us(500);

      if (abort_flag) return;
    }
  }
}
