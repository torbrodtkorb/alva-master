#define __AVR_DEV_LIB_NAME__ avr64db32
#include <uart.h>
#include <analog.h>
#include <height-stepper.h>
#include <rotation-stepper.h>
#include <limit-switch.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>

// μ-stepping rotation stepper:
//
// 400
// 800
// 1600
// 3200
// 6400
// 12800 <
// 25600
// 51200
//
// sample_count = μ-stepping * (60 / 14) * (19 / 17)
//                              ^           ^
//                              gear ratio  number of poles to sample
// sample_count = 12800 * (60 / 14) * (19 / 17)
//
// ADC timing:
// With 2 MHz ADC clock and 24 MHz peripheral clock the conversion time is:
// conversion time (s) = (2 + samplecount * 188) / 24000000
// conversion time (us) = (2 + 16 * 188) / 24 = 125.4 us
// conversion time (cycles) = (2 + 16 * 188) = 3010 ≈ 4000 cycles
//
// Pulse duration:
// pulse duration (us) = 40 us
// pulse duration (cycles) = 24 * 40 = 960 ≈ 1000 cycles
//
// UART timing:
// time to send one byte (s) = 1 / (baudrate / 8)
// time to send one sample (us) = 2 * 1.000.000 / (921600 / 8) = 17.36 ≈ 18 us ≈ 500 cycles

// u-stepping 12800
#define SAMPLE_COUNT 61311ul

// u-stepping 25600
//#define SAMPLE_COUNT 122622ul

// u-stepping 51200
//#define SAMPLE_COUNT 245244ul

static volatile int state;

static volatile uint32_t samples_remaining;
static volatile uint16_t sample;
static volatile bool     sampling;
static volatile bool     last_byte;
static volatile bool     send_upper;

void system_init() {
  ccp_write_io((uint8_t* )&CLKCTRL.MCLKCTRLB, 0);
  ccp_write_io((uint8_t* )&CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc);
  ccp_write_io((uint8_t* )&WDT.CTRLA, 0);
}

void rotation_stepper_callback () {
  if (samples_remaining) {
    samples_remaining--;
    analog_start();
  }
}

void analog_callback (uint16_t result) {
  if (!samples_remaining) {
    rotation_stepper_stop();
    last_byte = true;
  }

  sample = result;
  send_upper = true;
  uart_send(sample);
}

void uart_transmitted_callback () {
  if (sampling && send_upper) {
    send_upper = false;
    uart_send(sample >> 8);

    if (last_byte) {
      last_byte = false;
      sampling = false;
      uart_send_crc();
    }
  }
}

static void take_measurement () {
  samples_remaining = SAMPLE_COUNT;
  sampling = true;
  send_upper = false;
  last_byte = false;

  rotation_stepper_start();

  while (sampling) {
    if (abort_flag) {
      cli();
      samples_remaining = 0;
      sampling = true;
      send_upper = false;
      last_byte = false;
      analog_stop();
      rotation_stepper_stop();
      sei();
      break;
    }
  }
}

enum {
  STATE_IDLE,
  STATE_LIMIT_SWITCH,
  STATE_DOWN_FAST,
  STATE_DOWN_SLOW,
  STATE_MEASUREMENT,
  STATE_RESET_STATOR,
  STATE_UP_SLOW,
  STATE_SEND_COMPLETE,
  STATE_UP_FAST,
  STATE_ABORT,
};

static void abort_switch_init () {
  PORTF.DIRCLR = 1 << 2 | 1 << 3;
  PORTF.PIN2CTRL = PORT_ISC_FALLING_gc;
  PORTF.PIN3CTRL = PORT_ISC_FALLING_gc;
}

ISR (PORTF_PORT_vect) {
  if (PORTF.INTFLAGS & (1 << 2)) {
    if (state == STATE_IDLE)
      start_flag = true;
  } else if (PORTF.INTFLAGS & (1 << 3)) {
    abort_flag = true;
  }

  PORTF.INTFLAGS = PORTF.INTFLAGS;
}

int main () {
  system_init();
  uart_init();
  analog_init();
  height_stepper_init();
  rotation_stepper_init();
  limit_switch_init();
  abort_switch_init();
  sei();

  // When starting, make sure the limit switch is not pressed
  if (limit_switch_pressed())
    height_stepper_move(DIRECTION_DOWN, SPEED_FAST, 4000);

  uint16_t measurement_count = 0;

  while (1) {
    if (abort_flag)
      state = STATE_ABORT;

    switch (state) {
      case STATE_IDLE:
        if (start_flag) {
          state = STATE_LIMIT_SWITCH;
          start_flag = false;
        }
        break;

      case STATE_LIMIT_SWITCH:
        while (limit_switch_not_pressed())
          height_stepper_move(DIRECTION_UP, SPEED_SLOW, 1);
        state = STATE_DOWN_FAST;
        break;

      case STATE_DOWN_FAST:
        height_stepper_move(DIRECTION_DOWN, SPEED_FAST, single_sample ? 22672 : 27000);
        state = STATE_DOWN_SLOW;
        break;

      case STATE_DOWN_SLOW:
        height_stepper_move(DIRECTION_DOWN, SPEED_SLOW, 1000);
        state = STATE_MEASUREMENT;
        measurement_count = 0;
        break;

      case STATE_MEASUREMENT:
        uart_reset_crc();
        take_measurement();
        state = STATE_RESET_STATOR;
        break;
      
      case STATE_RESET_STATOR:
        rotation_stepper_move_back(SAMPLE_COUNT);
        uint16_t tmp = single_sample ? 20 : 11; // 11 can not be changed without changed the number of steps in STATE_UP_SLOW
        state = (++measurement_count == tmp) ? STATE_SEND_COMPLETE : STATE_UP_SLOW;
        break;

      case STATE_UP_SLOW:
        if (!single_sample)
          height_stepper_move(DIRECTION_UP, SPEED_SLOW, 866);
        state = STATE_MEASUREMENT;
        break;

      case STATE_SEND_COMPLETE:
        uart_send_done();
        state = STATE_UP_FAST;
        break;

      case STATE_UP_FAST:
        height_stepper_move(DIRECTION_UP, SPEED_FAST, single_sample ? 10672 : 15000);
        start_flag = false;
        state = STATE_IDLE;
        break;

      case STATE_ABORT:
        state = STATE_IDLE;
        start_flag = false;
        abort_flag = false;

        cli();
        while (limit_switch_not_pressed())
          height_stepper_move(DIRECTION_UP, SPEED_FAST, 1);
        height_stepper_move(DIRECTION_DOWN, SPEED_FAST, 4000);
        sei();
        uart_send_abort();
        break;
    }
  }

  return 0;
}