#include <analog.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void analog_init () {
  PORTD.DIRCLR = 1 << 5 | 1 << 6 | 1 << 7;
  VREF.ADC0REF = VREF_REFSEL_VREFA_gc;
  ADC0.CTRLB   = ADC_SAMPNUM_ACC16_gc;
  ADC0.CTRLC   = ADC_PRESC_DIV12_gc;
  ADC0.CTRLD   = ADC_INITDLY_DLY256_gc;
  ADC0.MUXPOS  = 5;
  ADC0.MUXNEG  = 6;
  ADC0.INTCTRL = ADC_RESRDY_bm;
  ADC0.CTRLA   = ADC_ENABLE_bm | ADC_CONVMODE_bm;
}

void analog_start () {
  ADC0.COMMAND = ADC_STCONV_bm;
}

void analog_stop () {
  ADC0.COMMAND = ADC_SPCONV_bm;
  (void)ADC0.RES;
}

ISR (ADC0_RESRDY_vect) {
  uint16_t result = ADC0.RES;
  analog_callback(result);
}
