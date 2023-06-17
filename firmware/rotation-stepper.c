#define __AVR_DEV_LIB_NAME__ avr64db32
#include <rotation-stepper.h>
#include <uart.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void rotation_stepper_init () {
  PORTD.DIRSET = 1 << 1 | 1 << 4;
  PORTD.PIN1CTRL = PORT_INVEN_bm;
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTD_gc;

  TCA0.SINGLE.CTRLB   = TCA_SINGLE_CMP1EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm;
  TCA0.SINGLE.CMP1    = 2000ul;
  TCA0.SINGLE.PER     = 4000ul;
}

void rotation_stepper_move_back (uint32_t steps) {
  PORTMUX.TCAROUTEA = 0;
  PORTD.OUTTGL = 1 << 4;

  for (uint32_t i = 0; i < steps; i++) {
    PORTD.OUTSET = 1 << 1;
    _delay_us(20);
    PORTD.OUTCLR = 1 << 1;
    _delay_us(20);

    if (abort_flag)
      break;
  }

  PORTD.OUTTGL = 1 << 4;
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTD_gc;
}

void rotation_stepper_start () {
  TCA0.SINGLE.CTRLESET = TCA_SINGLE_CMD_RESTART_gc;
  TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
}

void rotation_stepper_stop () {
  TCA0.SINGLE.CTRLA = 0;
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm; // Clear any interrupt flags
}

ISR (TCA0_CMP1_vect) {
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP1_bm;
  rotation_stepper_callback();
}
