#include <limit-switch.h>
#include <avr/io.h>

void limit_switch_init () {
  PORTC.DIRCLR = 1 << 0;
  PORTC.PIN0CTRL = PORT_PULLUPEN_bm;
}

bool limit_switch_pressed () {
  return (PORTC.IN & (1 << 0)) == 0;
}

bool limit_switch_not_pressed () {
  return !limit_switch_pressed();
}